/**
 * @file tkLayoutTracker_geo.cpp
 * @brief DD4hep detector constructor plugin for tkLayout-generated compact XML.
 *
 * Registered as detector type "tkLayout_Tracker".  The compact XML produced by
 * DD4hepCompactWriter drives what this plugin builds:
 *
 *   Phase 1 — envelope Assembly only; verifies the plugin loads correctly.
 *   Phase 2 — walks <shapes>, <volumes>, <placements> child elements to build
 *              the full explicit-placement geometry tree.
 *   Phase 3 — marks sensitive volumes and attaches a minimal DetElement tree.
 *
 * Build separately against DD4hep; see plugins/dd4hep/CMakeLists.txt.
 */

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"

using namespace dd4hep;

// ---------------------------------------------------------------------------
// Helper: build a 3×3 rotation matrix from a <rotation x=… y=… z=…*deg> element
// ---------------------------------------------------------------------------
static RotationZYX rotFromXml(xml_comp_t rot) {
  const double x = rot.hasAttr(_U(x)) ? rot.x() : 0.0;
  const double y = rot.hasAttr(_U(y)) ? rot.y() : 0.0;
  const double z = rot.hasAttr(_U(z)) ? rot.z() : 0.0;
  return RotationZYX(z, y, x);  // dd4hep: ZYX intrinsic convention
}

// ---------------------------------------------------------------------------
// Phase 2: materialise <shapes>, <volumes>, <placements> from compact XML
// ---------------------------------------------------------------------------

static void buildShapes(Detector& description, xml_h shapes_h,
                        std::map<std::string, Solid>& solidMap) {
  for (xml_coll_t c(shapes_h, _Unicode(box)); c; ++c) {
    xml_comp_t s(c);
    const std::string name = s.nameStr();
    solidMap[name] = Box(s.x()/2., s.y()/2., s.z()/2.);
    solidMap[name]->SetName(name.c_str());
  }
  for (xml_coll_t c(shapes_h, _Unicode(trd)); c; ++c) {
    xml_comp_t s(c);
    const std::string name = s.nameStr();
    solidMap[name] = Trapezoid(s.x1()/2., s.x2()/2.,
                                s.y1()/2., s.y2()/2.,
                                s.z() /2.);
    solidMap[name]->SetName(name.c_str());
  }
  for (xml_coll_t c(shapes_h, _Unicode(tube)); c; ++c) {
    xml_comp_t s(c);
    const std::string name = s.nameStr();
    solidMap[name] = Tube(s.rmin(), s.rmax(), s.dz());
    solidMap[name]->SetName(name.c_str());
  }
  for (xml_coll_t c(shapes_h, _Unicode(cone)); c; ++c) {
    xml_comp_t s(c);
    const std::string name = s.nameStr();
    solidMap[name] = Cone(s.dz(), s.rmin1(), s.rmax1(), s.rmin2(), s.rmax2());
    solidMap[name]->SetName(name.c_str());
  }
  for (xml_coll_t c(shapes_h, _Unicode(polycone)); c; ++c) {
    xml_comp_t pc(c);
    const std::string name = pc.nameStr();
    std::vector<double> z, rmin, rmax;
    for (xml_coll_t zp(pc, _Unicode(zplane)); zp; ++zp) {
      xml_comp_t zpc(zp);
      z   .push_back(zpc.z());
      rmin.push_back(zpc.rmin());
      rmax.push_back(zpc.rmax());
    }
    solidMap[name] = Polycone(0., 2.*M_PI, rmin, rmax, z);
    solidMap[name]->SetName(name.c_str());
  }
  // Boolean operations
  for (xml_coll_t c(shapes_h, _Unicode(union)); c; ++c) {
    xml_comp_t s(c);
    const std::string name = s.nameStr();
    xml_coll_t shapeChildren(s, _Unicode(shape));
    xml_comp_t s1(shapeChildren);
    ++shapeChildren;
    xml_comp_t s2(shapeChildren);
    Position pos(0,0,0);
    if (s2.hasChild(_Unicode(position))) {
      xml_comp_t p(s2.child(_Unicode(position)));
      pos = Position(p.x(), p.y(), p.z());
    }
    solidMap[name] = UnionSolid(solidMap.at(s1.attr<std::string>(_Unicode(ref))),
                                 solidMap.at(s2.attr<std::string>(_Unicode(ref))),
                                 Transform3D(pos));
    solidMap[name]->SetName(name.c_str());
  }
  for (xml_coll_t c(shapes_h, _Unicode(subtraction)); c; ++c) {
    xml_comp_t s(c);
    const std::string name = s.nameStr();
    xml_coll_t shapeChildren(s, _Unicode(shape));
    xml_comp_t s1(shapeChildren); ++shapeChildren;
    xml_comp_t s2(shapeChildren);
    Position pos(0,0,0);
    if (s2.hasChild(_Unicode(position))) {
      xml_comp_t p(s2.child(_Unicode(position)));
      pos = Position(p.x(), p.y(), p.z());
    }
    solidMap[name] = SubtractionSolid(solidMap.at(s1.attr<std::string>(_Unicode(ref))),
                                       solidMap.at(s2.attr<std::string>(_Unicode(ref))),
                                       Transform3D(pos));
    solidMap[name]->SetName(name.c_str());
  }
  for (xml_coll_t c(shapes_h, _Unicode(intersection)); c; ++c) {
    xml_comp_t s(c);
    const std::string name = s.nameStr();
    xml_coll_t shapeChildren(s, _Unicode(shape));
    xml_comp_t s1(shapeChildren); ++shapeChildren;
    xml_comp_t s2(shapeChildren);
    Position pos(0,0,0);
    if (s2.hasChild(_Unicode(position))) {
      xml_comp_t p(s2.child(_Unicode(position)));
      pos = Position(p.x(), p.y(), p.z());
    }
    solidMap[name] = IntersectionSolid(solidMap.at(s1.attr<std::string>(_Unicode(ref))),
                                        solidMap.at(s2.attr<std::string>(_Unicode(ref))),
                                        Transform3D(pos));
    solidMap[name]->SetName(name.c_str());
  }
}

static void buildVolumes(Detector& description, xml_h volumes_h,
                         const std::map<std::string, Solid>& solidMap,
                         std::map<std::string, Volume>& volMap) {
  for (xml_coll_t c(volumes_h, _Unicode(volume)); c; ++c) {
    xml_comp_t v(c);
    const std::string name   = v.nameStr();
    const std::string solid  = v.attr<std::string>(_Unicode(solid));
    const std::string matStr = v.attr<std::string>(_Unicode(material));
    Material mat = description.material(matStr);
    Volume vol(name, solidMap.at(solid), mat);
    volMap[name] = vol;
  }
}

static void buildPlacements(xml_h placements_h,
                             std::map<std::string, Volume>& volMap,
                             SensitiveDetector& sens) {
  for (xml_coll_t c(placements_h, _Unicode(placement)); c; ++c) {
    xml_comp_t p(c);
    const std::string parent  = p.attr<std::string>(_Unicode(parent));
    const std::string child   = p.attr<std::string>(_Unicode(child));
    const int         copyNo  = p.hasAttr(_Unicode(copyno))
                                  ? p.attr<int>(_Unicode(copyno)) : 0;
    const Position pos(
      p.hasAttr(_U(x)) ? p.x() : 0.,
      p.hasAttr(_U(y)) ? p.y() : 0.,
      p.hasAttr(_U(z)) ? p.z() : 0.);

    Rotation3D rot;
    if (p.hasChild(_Unicode(rotation))) {
      xml_comp_t r(p.child(_Unicode(rotation)));
      RotationZYX rzyx = rotFromXml(r);
      rot = Rotation3D(rzyx);
    }

    Volume parentVol = volMap.at(parent);
    Volume childVol  = volMap.at(child);
    parentVol.placeVolume(childVol, copyNo, Transform3D(rot, pos));
  }
  (void)sens; // used in Phase 3 for setSensitiveDetector
}

// ---------------------------------------------------------------------------
// Main entry point
// ---------------------------------------------------------------------------

static Ref_t create_detector(Detector& description, xml_h e, SensitiveDetector sens) {
  xml_det_t   x_det  = e;
  std::string detName = x_det.nameStr();
  int         detId   = x_det.id();

  DetElement det(detName, detId);
  Assembly   assembly(detName);

  // Phase 2: build geometry if the XML contains structural sub-elements.
  std::map<std::string, Solid>  solidMap;
  std::map<std::string, Volume> volMap;

  if (x_det.hasChild(_Unicode(shapes)))
    buildShapes(description, x_det.child(_Unicode(shapes)), solidMap);

  if (x_det.hasChild(_Unicode(volumes))) {
    buildVolumes(description, x_det.child(_Unicode(volumes)), solidMap, volMap);
    // Make all named volumes accessible from the assembly.
    volMap[detName] = assembly;
  }

  if (x_det.hasChild(_Unicode(placements)))
    buildPlacements(x_det.child(_Unicode(placements)), volMap, sens);

  // Place the top-level assembly in the world volume.
  PlacedVolume pv = description.pickMotherVolume(det).placeVolume(assembly);
  pv.addPhysVolID("system", detId);
  det.setPlacement(pv);

  printout(INFO, "tkLayoutTracker",
           "Detector '%s' (id=%d) placed — %zu shapes, %zu volumes",
           detName.c_str(), detId, solidMap.size(), volMap.size());

  return det;
}

DECLARE_DETELEMENT(tkLayout_Tracker, create_detector)
