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
  const double d = M_PI / 180.0;

  // Column vectors (world coords of the three rotated axes).
  const double xx = std::sin(r.thetax*d)*std::cos(r.phix*d);
  const double xy = std::sin(r.thetax*d)*std::sin(r.phix*d);
  const double xz = std::cos(r.thetax*d);

  const double yz = std::cos(r.thetay*d);

  const double zy = std::sin(r.thetaz*d)*std::sin(r.phiz*d);
  const double zz = std::cos(r.thetaz*d);

  // ZYX decomposition: R = Rz(α)·Ry(β)·Rx(γ)
  //   R[2][0] = -sin(β)           → β = asin(-R[2][0]) = asin(-xz)
  //   R[2][1] = cos(β)·sin(γ)     → γ = atan2(R[2][1], R[2][2])
  //   R[1][0] = sin(α)·cos(β)
  //   R[0][0] = cos(α)·cos(β)     → α = atan2(R[1][0], R[0][0])
  const double beta    = std::asin(-xz);
  const double cosBeta = std::cos(beta);

  double rotX, rotZ;
  if (std::fabs(cosBeta) > 1e-9) {
    rotZ = std::atan2(xy,  xx);  // α = atan2(R[1][0], R[0][0])
    rotX = std::atan2(yz,  zz);  // γ = atan2(R[2][1], R[2][2])
  } else {
    // Gimbal lock (β = ±90°): set γ = 0, solve for α.
    rotX = 0.0;
    rotZ = std::atan2(-zy, zz);
  }

  std::ostringstream os;
  os << std::fixed << std::setprecision(10);
  os << indent << "<rotation"
     << " x=\"" << rotX/d << "*deg\""
     << " y=\"" << beta/d << "*deg\""
     << " z=\"" << rotZ/d << "*deg\"/>\n";
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
  const std::string name = xml_tkLayout_material + e.tag;
  os << "    <element Z=\"" << e.atomic_number << "\""
     << " formula=\"" << e.tag << "\""
     << " name=\"" << name << "\">\n"
     << "      <atom unit=\"g/mol\" value=\"" << e.atomic_weight << "\"/>\n"
     << "    </element>\n";
}

void writeComposite(std::ostream& os, const Composite& c) {
  os << "    <material name=\"" << c.name << "\">\n"
     << "      <D type=\"density\" unit=\"g/cm3\" value=\"" << c.density << "\"/>\n";
  for (const auto& kv : c.elements) {
    os << "      <fraction n=\"" << kv.second
       << "\" ref=\"" << xml_tkLayout_material << kv.first << "\"/>\n";
  }
  os << "    </material>\n";
}

void writeMaterials(std::ostream& os,
                    std::vector<Element>&   elems,
                    std::vector<Composite>& comps) {
  os << "  <materials>\n";
  for (const auto& e : elems)
    writeElement(os, e);

  std::set<std::string> emitted;
  for (const auto& c : comps) {
    if (emitted.count(c.name)) continue;
    writeComposite(os, c);
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
      // rzup[i] = (r_outer, z), rzdown[i] = (r_inner, z); sections by increasing z.
      os << "        <polycone name=\"" << name
         << "\" startphi=\"0\" deltaphi=\"360*deg\">\n";
      const std::size_t n = s.rzup.size();
      for (std::size_t i = 0; i < n; ++i) {
        const double rmin = (i < s.rzdown.size()) ? s.rzdown[i].first : 0.0;
        os << "          <zplane z=\""    << s.rzup[i].second << "*mm\""
           << " rmin=\"" << rmin           << "*mm\""
           << " rmax=\"" << s.rzup[i].first << "*mm\"/>\n";
      }
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

  if (!d.logic.empty()) {
    os << "      <volumes>\n";
    for (const auto& l : d.logic) writeVolume(os, l);
    os << "      </volumes>\n";
  }

  if (!d.positions.empty() || !d.algos.empty() || !d.logic.empty()) {
    // Collect all child names so we can identify root volumes.
    std::set<std::string> childNames;
    for (const auto& p : d.positions)
      childNames.insert(stripNS(p.child_tag));
    for (const auto& a : d.algos) {
      const AlgoExpanded exp = parseAlgoParams(a);
      if (!exp.childName.empty())
        childNames.insert(exp.childName);
    }

    os << "      <placements>\n";
    // Root volumes (in logic but never a child) are placed into the assembly.
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
       << "  <includes>\n"
       << "    <gdmlFile ref=\"${DD4hepINSTALL}/DDCore/include/Legends.xml\"/>\n"
       << "  </includes>\n"
       << "\n"
       << "  <define/>\n"
       << "\n";

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
