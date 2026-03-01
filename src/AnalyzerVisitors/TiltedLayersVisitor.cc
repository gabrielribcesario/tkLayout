#include <string>

#include "RootWeb.hh"
#include "Layer.hh"
#include "VizardTools.hh"
#include "AnalyzerVisitors/TiltedLayersVisitor.hh"

    //***************************************//
    //*                Visitor              *//
    //* Automatic-placement tilted layers : *//
    //*            Additional info          *//
    //*                                     *//
    //***************************************//

/**
   Visits tilted layers and gathers info on its flat and tilted parts.
*/
void TiltedLayersVisitor::visit(const Layer& l) {
  // Only for tilted layers with automatic placement.
  if (l.isTilted() && l.isTiltedAuto()) {
    numTiltedLayers++;

    // Initializes layer name
    RootWTable* tiltedLayerName = new RootWTable();
    tiltedLayerName->setContent(0, 0, "Layer " + std::to_string(l.myid()) + " :");
    tiltedLayerNames.push_back(tiltedLayerName);

    // Initializes flat part name
    RootWTable* flatPartName = new RootWTable();
    flatPartName->setContent(0, 0, "Flat part :");
    flatPartNames.push_back(flatPartName);

    // Initializes tilted part name
    RootWTable* tiltedPartName = new RootWTable();
    tiltedPartName->setContent(0, 0, "Tilted part :");
    tiltedPartNames.push_back(tiltedPartName);

    // FILLS TILTED PART TABLE
    RootWTable* tiltedPartTable = new RootWTable();
    const int numTiltedRings = l.tiltedRingsGeometry().size();
    for (int i=0; i < numTiltedRings; i++) {
      int ringNumber = l.buildNumModulesFlat() + 1 + i;
      tiltedPartTable->setContent(0, 0, "Ring");
      tiltedPartTable->setContent(0, i+1, ringNumber);
      tiltedPartTable->setContent(1, 0, "tiltAngle (°)");
      tiltedPartTable->setContent(1, i+1, l.tiltedRingsGeometry()[ringNumber]->tiltAngle(), anglePrecision);
      tiltedPartTable->setContent(2, 0, "tiltAngleIdeal" + subStart + "Inner" + subEnd + " (°)");
      tiltedPartTable->setContent(2, i+1, l.tiltedRingsGeometry()[ringNumber]->tiltAngleIdealInner(), anglePrecision);
      tiltedPartTable->setContent(3, 0, "deltaTiltIdeal" + subStart + "Inner" + subEnd + " (°)");
      tiltedPartTable->setContent(3, i+1, l.tiltedRingsGeometry()[ringNumber]->deltaTiltIdealInner(), anglePrecision);
      tiltedPartTable->setContent(4, 0, "tiltAngleIdeal" + subStart + "Outer" + subEnd + " (°)");
      tiltedPartTable->setContent(4, i+1, l.tiltedRingsGeometry()[ringNumber]->tiltAngleIdealOuter(), anglePrecision);
      tiltedPartTable->setContent(5, 0, "deltaTiltIdeal" + subStart + "Outer" + subEnd + " (°)");
      tiltedPartTable->setContent(5, i+1, l.tiltedRingsGeometry()[ringNumber]->deltaTiltIdealOuter(), anglePrecision);
      tiltedPartTable->setContent(6, 0, "theta_g (°)");
      tiltedPartTable->setContent(6, i+1, l.tiltedRingsGeometry()[ringNumber]->theta_g(), anglePrecision);
      tiltedPartTable->setContent(7, 0, "r" + subStart + "Inner" + subEnd);
      tiltedPartTable->setContent(7, i+1, l.tiltedRingsGeometry()[ringNumber]->innerRadius(), coordPrecision);
      tiltedPartTable->setContent(8, 0, "r" + subStart + "Outer" + subEnd);
      tiltedPartTable->setContent(8, i+1, l.tiltedRingsGeometry()[ringNumber]->outerRadius(), coordPrecision);
      tiltedPartTable->setContent(9, 0, "averageR (on Ring)");
      tiltedPartTable->setContent(9, i+1, l.tiltedRingsGeometry()[ringNumber]->averageR(), coordPrecision);
      tiltedPartTable->setContent(10, 0, "gapR");
      tiltedPartTable->setContent(10, i+1, l.tiltedRingsGeometry()[ringNumber]->gapR(), coordPrecision);
      tiltedPartTable->setContent(11, 0, "z" + subStart + "Inner" + subEnd);
      tiltedPartTable->setContent(11, i+1, l.tiltedRingsGeometry()[ringNumber]->zInner(), coordPrecision);
      tiltedPartTable->setContent(12, 0, "z" + subStart + "Outer" + subEnd);
      tiltedPartTable->setContent(12, i+1, l.tiltedRingsGeometry()[ringNumber]->zOuter(), coordPrecision);
      tiltedPartTable->setContent(13, 0, "averageZ (on Ring)");
      tiltedPartTable->setContent(13, i+1, l.tiltedRingsGeometry()[ringNumber]->averageZ(), coordPrecision);
      tiltedPartTable->setContent(14, 0, "deltaZ" + subStart + "Inner" + subEnd + " (Ring i & i-1)");
      tiltedPartTable->setContent(14, i+1, l.tiltedRingsGeometryInfo().deltaZInner()[ringNumber], coordPrecision);
      tiltedPartTable->setContent(15, 0, "deltaZ" + subStart + "Outer" + subEnd + " (Ring i & i-1)");
      tiltedPartTable->setContent(15, i+1, l.tiltedRingsGeometryInfo().deltaZOuter()[ringNumber], coordPrecision);
      tiltedPartTable->setContent(16, 0, "phiOverlap");
      tiltedPartTable->setContent(16, i+1, l.tiltedRingsGeometry()[ringNumber]->phiOverlap(), coordPrecision);
      tiltedPartTable->setContent(17, 0, "zOverlap" + subStart + "Outer" + subEnd);
      tiltedPartTable->setContent(17, i+1, l.tiltedRingsGeometry()[ringNumber]->ringZOverlap(), zOverlapPrecision);
      double zErrorInner = l.tiltedRingsGeometryInfo().zErrorInner()[ringNumber];
      tiltedPartTable->setContent(18, 0, "zError" + subStart + "Inner" + subEnd + " (Ring i & i-1)");
      if (!std::isnan(zErrorInner)) tiltedPartTable->setContent(18, i+1, zErrorInner, coordPrecision);
      else tiltedPartTable->setContent(18, i+1, "n/a");
      double zErrorOuter = l.tiltedRingsGeometryInfo().zErrorOuter()[ringNumber];
      tiltedPartTable->setContent(19, 0, "zError" + subStart + "Outer" + subEnd + " (Ring i & i-1)");
      if (!std::isnan(zErrorOuter)) tiltedPartTable->setContent(19, i+1, zErrorOuter, coordPrecision);
      else tiltedPartTable->setContent(19, i+1, "n/a");
    }
    tiltedPartTables.push_back(tiltedPartTable);

    // FILLS FLAT PART TABLE
    RootWTable* flatPartTable = new RootWTable();

    StraightRodPair* minusBigDeltaRod = (l.bigParity() > 0 ? l.flatPartRods().at(1) : l.flatPartRods().front());
    const auto& minusBigDeltaModules = minusBigDeltaRod->modules().first;
    StraightRodPair* plusBigDeltaRod = (l.bigParity() > 0 ? l.flatPartRods().front() : l.flatPartRods().at(1));
    const auto& plusBigDeltaModules = plusBigDeltaRod->modules().first;

    int i = 0;
    for (const auto& m : minusBigDeltaModules) {
      int ringNumber = i + 1;
      flatPartTable->setContent(0, 0, "Ring");
      flatPartTable->setContent(0, i+1, ringNumber);
      flatPartTable->setContent(1, 0, "r" + subStart + "Inner" + subEnd);
      flatPartTable->setContent(1, i+1, m.center().Rho(), coordPrecision);
      flatPartTable->setContent(3, 0, "averageR (on Flat part)");
      flatPartTable->setContent(3, i+1, l.flatPartAverageR(), coordPrecision);
      flatPartTable->setContent(4, 0, "bigDelta");
      flatPartTable->setContent(4, i+1, l.bigDelta(), coordPrecision);
      flatPartTable->setContent(5, 0, "smallDelta");
      flatPartTable->setContent(5, i+1, l.smallDelta(), coordPrecision);
      flatPartTable->setContent(6, 0, "z");
      flatPartTable->setContent(6, i+1, m.center().Z(), coordPrecision);
      flatPartTable->setContent(7, 0, "phiOverlap");
      flatPartTable->setContent(7, i+1, (((minusBigDeltaRod->zPlusParity() * pow(-1, (i%2))) > 0) ? l.flatPartPhiOverlapSmallDeltaPlus() : l.flatPartPhiOverlapSmallDeltaMinus()), coordPrecision);
      // In case beamSpotCover == false, zOverlap is the only parameter used as a Z-coverage constraint in the geometry construction process.
      // (There is then no zError taken into account).
      // As a result, it is interesting to display zOverlap !
      int extraLine = 0;
      if (!l.flatPartRods().front()->beamSpotCover()) {
	extraLine = 1;
	flatPartTable->setContent(8, 0, "zOverlap");
	flatPartTable->setContent(8, i+1, l.flatPartRods().front()->zOverlap(), coordPrecision);
	// WARNING : zOverlap, in the geometry construction process, is one value common for all flat part (or straight rod)
      }
      // calculates zError for ring (i) with respect to ring (i-1)
      if (i > 0) {
	// Inner modules in the tilted rings
	double zErrorInner = l.flatRingsGeometryInfo().zErrorInner()[i];
	flatPartTable->setContent(8 + extraLine, 0, "zError" + subStart + "Inner" + subEnd + " (Ring i & i-1)");
	if (!std::isnan(zErrorInner)) flatPartTable->setContent(8 + extraLine, i+1, zErrorInner, coordPrecision);
	else flatPartTable->setContent(8 + extraLine, i+1, "n/a");
	// Outer modules in the tilted rings
	double zErrorOuter = l.flatRingsGeometryInfo().zErrorOuter()[i];
	flatPartTable->setContent(9 + extraLine, 0, "zError" + subStart + "Outer" + subEnd + " (Ring i & i-1)");
	if (!std::isnan(zErrorOuter)) flatPartTable->setContent(9 + extraLine, i+1, zErrorOuter, coordPrecision);
	else flatPartTable->setContent(9 + extraLine, i+1, "n/a");
      }
      i++;
    }
    i = 0;
    for (const auto& m : plusBigDeltaModules) {
      flatPartTable->setContent(2, 0, "r" + subStart + "Outer" + subEnd);
      flatPartTable->setContent(2, i+1, m.center().Rho(), coordPrecision);
      i++;
    }
    flatPartTables.push_back(flatPartTable);
  } // end of 'fills flat part table'
}
