/**
 * @file DD4hepCompactWriter.cc
 * @brief Writes a native DD4hep compact XML (<lccdd>) from the CMSSWBundle IR.
 *
 * Phase 1: emits <info>, <materials>, a skeleton <detectors> envelope, and
 *          minimal <readouts>.  Shape/volume/placement tree activates in Phase 2
 *          once the IR vectors are non-empty.
 */

#include <DD4hepCompactWriter.hh>
#include <tk2CMSSW_datatypes.hh>
#include <tk2CMSSW_strings.hh>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Pull the IR types into the anonymous namespace so the helpers below can use
// them without verbose qualification.
using insur::AlgoInfo;
using insur::CMSSWBundle;
using insur::Composite;
using insur::CompType;
using insur::Element;
using insur::LogicalInfo;
using insur::PosInfo;
using insur::Rotation;
using insur::ShapeInfo;
using insur::ShapeOperationInfo;
using insur::ShapeOperationType;
using insur::ShapeType;
using insur::XmlTags;
using insur::xml_tkLayout_material;

// ---------------------------------------------------------------------------
// Internal helpers (translation-unit local)
// ---------------------------------------------------------------------------

namespace {

// Strip the "namespace:" prefix that DDD uses (e.g. "tracker:SomeVolume" → "SomeVolume").
std::string stripNS(const std::string& s) {
  const auto pos = s.find(':');
  return (pos == std::string::npos) ? s : s.substr(pos + 1);
}

// ---------------------------------------------------------------------------
// Rotation conversion: axis-image spherical angles (DDD) → ZYX Tait-Bryan
// ---------------------------------------------------------------------------
// The DDD Rotation struct stores the world-frame directions of the rotated axes:
//   thetaX/phiX → spherical angles of the rotated X axis
//   thetaY/phiY → spherical angles of the rotated Y axis
//   thetaZ/phiZ → spherical angles of the rotated Z axis  (degrees)
//
// We reconstruct the 3×3 matrix (columns = rotated axis directions) and
// decompose it into ZYX intrinsic Tait-Bryan angles that DD4hep understands.
std::string rotXML(const Rotation& r, const std::string& indent) {
  constexpr double RAD_TO_DEG = M_PI / 180.0;

  // X vector components (Column 0)
  const double xx = std::sin(r.thetax * RAD_TO_DEG) * std::cos(r.phix * RAD_TO_DEG);
  const double xy = std::sin(r.thetax * RAD_TO_DEG) * std::sin(r.phix * RAD_TO_DEG);
  
  // Y vector components (Column 1)
  const double yx = std::sin(r.thetay * RAD_TO_DEG) * std::cos(r.phiy * RAD_TO_DEG);
  const double yy = std::sin(r.thetay * RAD_TO_DEG) * std::sin(r.phiy * RAD_TO_DEG);
  
  // Z vector components (Column 2)
  const double zx = std::sin(r.thetaz * RAD_TO_DEG) * std::cos(r.phiz * RAD_TO_DEG);
  const double zy = std::sin(r.thetaz * RAD_TO_DEG) * std::sin(r.phiz * RAD_TO_DEG);
  const double zz = std::cos(r.thetaz * RAD_TO_DEG);

  // Extrinsic X-Y-Z matrix extraction (DD4hep/ROOT standard)
  // Clamp zx to [-1.0, 1.0] to prevent floating point inaccuracy causing NaN
  const double rotY_rad = std::asin(std::max(-1.0, std::min(1.0, zx))); 
  const double cosY = std::cos(rotY_rad);

  double rotX_rad, rotZ_rad;
  if (std::fabs(cosY) > 1e-9) {
    // Normal case
    rotX_rad = std::atan2(-zy, zz);
    rotZ_rad = std::atan2(-yx, xx);
  } else {
    // Gimbal lock (y is +/- 90 degrees)
    rotZ_rad = 0.0;
    if (zx > 0.0) { // y = +90
      rotX_rad = std::atan2(xy, yy);
    } else {        // y = -90
      rotX_rad = std::atan2(-xy, yy);
    }
  }

  std::ostringstream os;
  os << std::fixed << std::setprecision(10);
  os << indent << "<rotation"
     << " x=\"" << rotX_rad / RAD_TO_DEG << "*deg\""
     << " y=\"" << rotY_rad / RAD_TO_DEG << "*deg\""
     << " z=\"" << rotZ_rad / RAD_TO_DEG << "*deg\"/>\n";
  return os.str();
}

// ---------------------------------------------------------------------------
// Expanded representation of one DDTrackerXYZPosAlgo block.
// ---------------------------------------------------------------------------
struct AlgoExpanded {
  std::string childName;
  int         startCopyNo = 0;
  std::vector<double>      xpos, ypos, zpos;
  std::vector<std::string> rotations; // rotation name (NS stripped) or "NULL"
};

// Parse the pre-serialised DDL parameter strings stored in AlgoInfo::parameters.
//
// Format of each string (one parameter per entry):
//   <String  name="ChildName"   value="ns:volname"/>
//   <Numeric name="StartCopyNo" value="0"/>
//   <Vector  name="XPositions"  type="numeric" nEntries="1">100.5</Vector>
//   <Vector  name="Rotations"   type="string"  nEntries="1">ns:RotName</Vector>
AlgoExpanded parseAlgoParams(const AlgoInfo& a) {
  AlgoExpanded exp;

  for (const auto& param : a.parameters) {
    {
      static const std::regex re(R"RE(<String\s+name="ChildName"\s+value="([^"]+)"\s*/>)RE");
      std::smatch m;
      if (std::regex_search(param, m, re)) {
        exp.childName = stripNS(m[1].str());
        continue;
      }
    }
    {
      static const std::regex re(R"RE(<Numeric\s+name="StartCopyNo"\s+value="([^"]+)"\s*/>)RE");
      std::smatch m;
      if (std::regex_search(param, m, re)) {
        exp.startCopyNo = std::stoi(m[1].str());
        continue;
      }
    }
    {
      static const std::regex re(
        R"RE(<Vector\s+name="(XPositions|YPositions|ZPositions)"\s+type="numeric"[^>]*>(.*?)</Vector>)RE");
      std::smatch m;
      if (std::regex_search(param, m, re)) {
        const std::string which  = m[1].str();
        const std::string values = m[2].str();
        std::istringstream ss(values);
        std::string tok;
        std::vector<double> parsed;
        while (std::getline(ss, tok, ',')) {
          if (!tok.empty()) parsed.push_back(std::stod(tok));
        }
        if      (which == "XPositions") exp.xpos = std::move(parsed);
        else if (which == "YPositions") exp.ypos = std::move(parsed);
        else                            exp.zpos = std::move(parsed);
        continue;
      }
    }
    {
      static const std::regex re(
        R"RE(<Vector\s+name="Rotations"\s+type="string"[^>]*>(.*?)</Vector>)RE");
      std::smatch m;
      if (std::regex_search(param, m, re)) {
        const std::string values = m[1].str();
        std::istringstream ss(values);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
          if (!tok.empty()) exp.rotations.push_back(stripNS(tok));
        }
        continue;
      }
    }
  }

  // Pad shorter position arrays to match the longest.
  const std::size_t n = std::max({exp.xpos.size(), exp.ypos.size(), exp.zpos.size()});
  exp.xpos.resize(n, 0.0);
  exp.ypos.resize(n, 0.0);
  exp.zpos.resize(n, 0.0);
  return exp;
}

// ---------------------------------------------------------------------------
// Section writers
// ---------------------------------------------------------------------------

void writeInfo(std::ostream& os, const std::string& detName) {
  os << "  <info name=\"tkLayout_" << detName << "\"\n"
     << "        title=\"CMS " << detName << " generated by tkLayout\"\n"
     << "        author=\"tkLayout\"\n"
     << "        url=\"\"\n"
     << "        status=\"development\"\n"
     << "        version=\"1.0\">\n"
     << "    <comment>Compact geometry produced by tkLayout DD4hep exporter.</comment>\n"
     << "  </info>\n";
}

void writeElement(std::ostream& os, const Element& e) {
  // The underlying <element> gets a "_elem" suffix so it doesn't clash with the
  // companion <material> of the same base name.  Volumes reference the <material>
  // via description.material(); composites reference the <element> for fractions.
  const std::string elemName = xml_tkLayout_material + e.tag + "_elem";
  const std::string matName  = xml_tkLayout_material + e.tag;
  os << "    <element Z=\"" << e.atomic_number << "\""
     << " formula=\"" << e.tag << "\""
     << " name=\"" << elemName << "\">\n"
     << "      <atom unit=\"g/mol\" value=\"" << e.atomic_weight << "\"/>\n"
     << "    </element>\n";
  // Pure-element material so <volume material="tkLayout_X"> resolves correctly.
  os << "    <material name=\"" << matName << "\">\n"
     << "      <D type=\"density\" unit=\"g/cm3\" value=\"" << e.density << "\"/>\n"
     << "      <fraction n=\"1.0\" ref=\"" << elemName << "\"/>\n"
     << "    </material>\n";
}

void writeComposite(std::ostream& os, const Composite& c,
                    const std::set<std::string>& elemTags) {
  os << "    <material name=\"" << c.name << "\">\n"
     << "      <D type=\"density\" unit=\"g/cm3\" value=\"" << c.density << "\"/>\n";
  for (const auto& kv : c.elements) {
    // Chemical elements get the _elem suffix (their <element> node is renamed);
    // sub-composite keys reference the composite <material> directly.
    const std::string ref = elemTags.count(kv.first)
                            ? xml_tkLayout_material + kv.first + "_elem"
                            : xml_tkLayout_material + kv.first;
    os << "      <fraction n=\"" << kv.second << "\" ref=\"" << ref << "\"/>\n";
  }
  os << "    </material>\n";
}

// Standard DD4hep materials normally supplied by Legends.xml.
// Emitted here so the XML is self-contained after dropping that include.
void writeStdMaterials(std::ostream& os) {
  os << "    <!-- standard materials -->\n"
     << "    <element Z=\"1\"  formula=\"H\"  name=\"H\" ><atom unit=\"g/mol\" value=\"1.008\"/></element>\n"
     << "    <element Z=\"7\"  formula=\"N\"  name=\"N\" ><atom unit=\"g/mol\" value=\"14.007\"/></element>\n"
     << "    <element Z=\"8\"  formula=\"O\"  name=\"O\" ><atom unit=\"g/mol\" value=\"15.999\"/></element>\n"
     << "    <material name=\"Vacuum\">\n"
     << "      <D type=\"density\" unit=\"g/cm3\" value=\"1e-25\"/>\n"
     << "      <fraction n=\"1.0\" ref=\"H\"/>\n"
     << "    </material>\n"
     << "    <material name=\"Air\">\n"
     << "      <D type=\"density\" unit=\"g/cm3\" value=\"1.2e-3\"/>\n"
     << "      <fraction n=\"0.7\" ref=\"N\"/>\n"
     << "      <fraction n=\"0.3\" ref=\"O\"/>\n"
     << "    </material>\n";
}

void writeMaterials(std::ostream& os,
                    std::vector<Element>&   elems,
                    std::vector<Composite>& comps) {
  std::set<std::string> elemTags;
  for (const auto& e : elems) elemTags.insert(e.tag);

  os << "  <materials>\n";
  writeStdMaterials(os);
  for (const auto& e : elems)
    writeElement(os, e);

  std::set<std::string> emitted;
  for (const auto& c : comps) {
    if (emitted.count(c.name)) continue;
    writeComposite(os, c, elemTags);
    emitted.insert(c.name);
  }
  os << "  </materials>\n";
}

void writeShape(std::ostream& os, const ShapeInfo& s) {
  const std::string name = stripNS(s.name_tag);
  switch (s.type) {
    case ShapeType::bx:
      os << "        <box name=\"" << name << "\""
         << " x=\"" << 2*s.dx << "*mm\""
         << " y=\"" << 2*s.dy << "*mm\""
         << " z=\"" << 2*s.dz << "*mm\"/>\n";
      break;
    case ShapeType::tb:
      os << "        <tube name=\"" << name << "\""
         << " rmin=\"" << s.rmin << "*mm\""
         << " rmax=\"" << s.rmax << "*mm\""
         << " dz=\""   << s.dz   << "*mm\"/>\n";
      break;
    case ShapeType::co:
      os << "        <cone name=\"" << name << "\""
         << " rmin1=\"" << s.rmin1 << "*mm\""
         << " rmax1=\"" << s.rmax1 << "*mm\""
         << " rmin2=\"" << s.rmin2 << "*mm\""
         << " rmax2=\"" << s.rmax2 << "*mm\""
         << " dz=\""    << s.dz    << "*mm\"/>\n";
      break;
    case ShapeType::tp:
      // DD4hep TGeoTrd2: x1/x2 are half-widths in X at -z/+z faces; same for y.
      // DDD Trd1 stores dx(bottom)/dxx(top) and dy/dyy — direct mapping.
      os << "        <trd name=\"" << name << "\""
         << " x1=\"" << s.dx  << "*mm\""
         << " x2=\"" << s.dxx << "*mm\""
         << " y1=\"" << s.dy  << "*mm\""
         << " y2=\"" << s.dyy << "*mm\""
         << " z=\""  << s.dz  << "*mm\"/>\n";
      break;
    case ShapeType::pc: {
      // rzup[i] = (r_outer, z), rzdown[i] = (r_inner, z).
      // Collect, sort by z ascending, then emit.
      struct ZPlane { double z, rmin, rmax; };
      std::vector<ZPlane> planes;
      planes.reserve(s.rzup.size());
      for (std::size_t i = 0; i < s.rzup.size(); ++i) {
        const double rmin = (i < s.rzdown.size()) ? s.rzdown[i].first : 0.0;
        planes.push_back({s.rzup[i].second, rmin, s.rzup[i].first});
      }
      std::sort(planes.begin(), planes.end(),
                [](const ZPlane& a, const ZPlane& b){ return a.z < b.z; });
      // Nudge the outermost planes outward by 1 nm so they are always
      // mathematically distinct from their neighbours even when the source
      // data places them at the same z.
      constexpr double kEps = 1e-6; // mm  (1 nm)
      if (planes.size() >= 2) {
        planes.front().z -= kEps;
        planes.back() .z += kEps;
      }
      os << "        <polycone name=\"" << name
         << "\" startphi=\"0\" deltaphi=\"360*deg\">\n";
      for (const auto& zp : planes)
        os << "          <zplane z=\""    << zp.z    << "*mm\""
           << " rmin=\"" << zp.rmin << "*mm\""
           << " rmax=\"" << zp.rmax << "*mm\"/>\n";
      os << "        </polycone>\n";
      break;
    }
    default:
      std::cerr << "DD4hepCompactWriter::writeShape: unknown type for " << name << "\n";
  }
}

void writeShapeOp(std::ostream& os, const ShapeOperationInfo& so) {
  std::string tag;
  switch (so.type) {
    case ShapeOperationType::uni:       tag = "union";        break;
    case ShapeOperationType::intersec:  tag = "intersection"; break;
    case ShapeOperationType::substract: tag = "subtraction";  break;
    default:                            tag = "union";
  }
  const std::string name = stripNS(so.name_tag);
  const std::string s1   = stripNS(so.rSolid1);
  const std::string s2   = stripNS(so.rSolid2);
  os << "        <" << tag << " name=\"" << name << "\">\n"
     << "          <shape ref=\"" << s1 << "\"/>\n"
     << "          <shape ref=\"" << s2 << "\">\n"
     << "            <position x=\"" << so.trans.dx << "*mm\""
     << " y=\"" << so.trans.dy << "*mm\""
     << " z=\"" << so.trans.dz << "*mm\"/>\n"
     << "          </shape>\n"
     << "        </" << tag << ">\n";
}

void writeVolume(std::ostream& os, const LogicalInfo& l) {
  const std::string volName  = stripNS(l.name_tag);
  const std::string solidRef = stripNS(l.shape_tag);
  const std::string matName  = stripNS(l.material_tag);
  os << "        <volume name=\"" << volName << "\""
     << " solid=\""    << solidRef << "\""
     << " material=\"" << matName  << "\"/>\n";
}

void writePlacement(std::ostream& os, const PosInfo& p,
                    const std::map<std::string, Rotation>& rots) {
  const std::string parent = stripNS(p.parent_tag);
  const std::string child  = stripNS(p.child_tag);

  os << "        <placement"
     << " parent=\"" << parent << "\""
     << " child=\""  << child  << "\""
     << " copyno=\"" << p.copy << "\""
     << " x=\"" << p.trans.dx << "*mm\""
     << " y=\"" << p.trans.dy << "*mm\""
     << " z=\"" << p.trans.dz << "*mm\"";

  if (!p.rotref.empty() && p.rotref != "NULL") {
    const std::string rotName = stripNS(p.rotref);
    auto it = rots.find(rotName);
    if (it != rots.end()) {
      os << ">\n" << rotXML(it->second, "          ")
         << "        </placement>\n";
      return;
    }
  }
  os << "/>\n";
}

void writeAlgoPlacements(std::ostream& os, const AlgoInfo& a,
                          const std::map<std::string, Rotation>& rots) {
  const AlgoExpanded exp    = parseAlgoParams(a);
  const std::string  parent = stripNS(a.parent);
  const std::size_t  n      = exp.xpos.size();

  for (std::size_t i = 0; i < n; ++i) {
    const int copyNo = exp.startCopyNo + static_cast<int>(i);
    os << "        <placement"
       << " parent=\""  << parent        << "\""
       << " child=\""   << exp.childName << "\""
       << " copyno=\""  << copyNo        << "\""
       << " x=\"" << exp.xpos[i] << "*mm\""
       << " y=\"" << exp.ypos[i] << "*mm\""
       << " z=\"" << exp.zpos[i] << "*mm\"";

    const std::string rotRef = (i < exp.rotations.size()) ? exp.rotations[i] : "NULL";
    if (rotRef != "NULL") {
      auto it = rots.find(rotRef);
      if (it != rots.end()) {
        os << ">\n" << rotXML(it->second, "          ")
           << "        </placement>\n";
        continue;
      }
    }
    os << "/>\n";
  }
}

void writeDetector(std::ostream& os, CMSSWBundle& d,
                   bool isPixelTracker, XmlTags& /*tags*/) {
  const std::string detName = isPixelTracker ? "InnerTracker" : "OuterTracker";
  const std::string readout = isPixelTracker ? "ITTrackerHits" : "OTTrackerHits";
  const int         detId   = isPixelTracker ? 2 : 1;

  os << "  <detectors>\n"
     << "    <detector id=\""   << detId   << "\""
     << " name=\""              << detName << "\""
     << " type=\"tkLayout_Tracker\""
     << " readout=\""           << readout << "\">\n"
     << "      <!--\n"
     << "        Phase 1: envelope only.\n"
     << "        Phase 2 adds <shapes>, <volumes>, <placements>.\n"
     << "      -->\n";

  if (!d.shapes.empty() || !d.shapeOps.empty()) {
    os << "      <shapes>\n";
    for (const auto& s  : d.shapes)   writeShape(os, s);
    for (const auto& so : d.shapeOps) writeShapeOp(os, so);
    os << "      </shapes>\n";
  }

  if (!d.positions.empty() || !d.algos.empty() || !d.logic.empty()) {
    // Build the sets needed for root-volume and phantom-parent detection.
    std::set<std::string> definedVols;
    for (const auto& l : d.logic)
      definedVols.insert(stripNS(l.name_tag));

    std::set<std::string> childNames;
    for (const auto& p : d.positions)
      childNames.insert(stripNS(p.child_tag));
    for (const auto& a : d.algos) {
      const AlgoExpanded exp = parseAlgoParams(a);
      if (!exp.childName.empty())
        childNames.insert(exp.childName);
    }

    // Phantom parents: referenced as a parent in positions/algos but never
    // defined in <volumes>.  These are DDD skeleton containers (Phase2OTBarrel,
    // Phase2OTForward, …) that don't exist in the tkLayout IR.  Emit them as
    // solid-less assembly volumes so the plugin can build an Assembly for each.
    std::set<std::string> phantomParents;
    for (const auto& p : d.positions) {
      const std::string parent = stripNS(p.parent_tag);
      if (!definedVols.count(parent) && parent != detName)
        phantomParents.insert(parent);
    }
    for (const auto& a : d.algos) {
      const std::string parent = stripNS(a.parent);
      if (!definedVols.count(parent) && parent != detName)
        phantomParents.insert(parent);
    }

    os << "      <volumes>\n";
    for (const auto& l : d.logic) writeVolume(os, l);
    for (const auto& name : phantomParents)
      os << "        <volume name=\"" << name << "\" assembly=\"true\"/>\n";
    os << "      </volumes>\n";

    os << "      <placements>\n";
    // Phantom parents are placed directly into the detector assembly.
    for (const auto& name : phantomParents)
      os << "        <placement parent=\"" << detName << "\""
         << " child=\"" << name << "\""
         << " copyno=\"1\" x=\"0*mm\" y=\"0*mm\" z=\"0*mm\"/>\n";
    // True root volumes (defined, but never a child) also go into the assembly.
    for (const auto& l : d.logic) {
      const std::string name = stripNS(l.name_tag);
      if (!childNames.count(name)) {
        os << "        <placement parent=\"" << detName << "\""
           << " child=\""  << name << "\""
           << " copyno=\"1\" x=\"0*mm\" y=\"0*mm\" z=\"0*mm\"/>\n";
      }
    }
    for (const auto& p : d.positions)
      writePlacement(os, p, d.rots);
    for (const auto& a : d.algos)
      writeAlgoPlacements(os, a, d.rots);
    os << "      </placements>\n";
  }

  os << "    </detector>\n"
     << "  </detectors>\n";
}

void writeReadouts(std::ostream& os, bool isPixelTracker) {
  const std::string readout = isPixelTracker ? "ITTrackerHits" : "OTTrackerHits";
  os << "  <readouts>\n"
     << "    <readout name=\"" << readout << "\">\n"
     << "      <id>system:8,barrel:3,layer:4,module:12,sensor:1</id>\n"
     << "    </readout>\n"
     << "  </readouts>\n";
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// DD4hepCompactWriter public implementation
// ---------------------------------------------------------------------------

namespace insur {

void DD4hepCompactWriter::tracker(CMSSWBundle& d, std::ofstream& out,
                                   bool isPixelTracker, XmlTags& tags) {
  if (!out.good())
    throw std::runtime_error("DD4hepCompactWriter::tracker: output stream is bad");

  const std::string detName = isPixelTracker ? "InnerTracker" : "OuterTracker";

  std::ostringstream body;
  body << std::fixed << std::setprecision(6);

  writeInfo(body, detName);

  body << "\n"
       << "  <define>\n"
       << "    <constant name=\"world_side\" value=\"10*m\"/>\n"
       << "    <constant name=\"world_x\"    value=\"world_side/2\"/>\n"
       << "    <constant name=\"world_y\"    value=\"world_side/2\"/>\n"
       << "    <constant name=\"world_z\"    value=\"world_side/2\"/>\n"
       << "  </define>\n\n";

  writeMaterials(body, d.elements, d.composites);

  body << "\n  <display/>\n\n";
  writeDetector(body, d, isPixelTracker, tags);
  body << "\n";
  writeReadouts(body, isPixelTracker);

  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  if (!simpleHeader_.empty())
    out << "<!--\n" << simpleHeader_ << "-->\n";
  out << "<lccdd>\n\n" << body.str() << "\n</lccdd>\n";

  if (out.fail())
    throw std::runtime_error("DD4hepCompactWriter::tracker: write error");
}

} // namespace insur
