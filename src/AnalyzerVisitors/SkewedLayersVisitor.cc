#include <string>
#include <cmath>

#include "VizardTools.hh"
#include "RootWeb.hh"
#include "Layer.hh"
#include "AnalyzerVisitors/SkewedLayersVisitor.hh"

    //***************************************//
    //*                Visitor              *//
    //*             Skewed layers:          *//
    //*            Additional info          *//
    //*                                     *//
    //***************************************//
/**
   Visits skewed layers.
*/
void SkewedLayersVisitor::visit(const Layer& l) {
  // Only for skewed layers.
  if (l.isSkewedForInstallation()) {
    numSkewedLayers++;

    // FILLS LAYER TABLE
    RootWTable* layerTable = new RootWTable();
    layerTable->setContent(0, 0, "Layer " + std::to_string(l.myid()) + " :");
    layerTable->setContent(1, 0, "layer Rho [mm]");
    layerTable->setContent(1, 1, l.placeRadiusHint(), coordPrecision);
    layerTable->setContent(2, 0, "bigDelta [mm]");
    layerTable->setContent(2, 1, l.bigDelta(), coordPrecision);
    layerTable->setContent(3, 0, "smallDelta [mm]");
    layerTable->setContent(3, 1, l.smallDelta(), coordPrecision);
    layerTable->setContent(4, 0, "inner module Rho" + subStart + "center" + subEnd + " [mm]");
    layerTable->setContent(4, 1, l.placeRadiusHint() - l.bigDelta(), coordPrecision);
    layerTable->setContent(5, 0, "outer module Rho" + subStart + "center" + subEnd + " [mm]");
    layerTable->setContent(5, 1, l.placeRadiusHint() + l.bigDelta(), coordPrecision);
    layerTable->setContent(6, 0, "skewed module Rho" + subStart + "center" + subEnd + " [mm]");
    layerTable->setContent(6, 1, l.skewedModuleCenterRho(), coordPrecision);
    layerTable->setContent(7, 0, "skewed module Rho" + subStart + "min" + subEnd + " [mm]");
    layerTable->setContent(7, 1, l.skewedModuleMinRho(), coordPrecision);
    layerTable->setContent(8, 0, "skewed module Rho" + subStart + "max" + subEnd + " [mm]");
    layerTable->setContent(8, 1, l.skewedModuleMaxRho(), coordPrecision);
    layerTable->setContent(9, 0, "skewed module edge Shift [mm]");
    layerTable->setContent(9, 1, l.skewedModuleEdgeShift(), coordPrecision);
    layerTable->setContent(10, 0, "skew angle [°]");
    layerTable->setContent(10, 1, l.skewAngle() * tkLayout::RAD_TO_DEG, coordPrecision);
    layerTable->setContent(11, 0, "phiOverlap [mm]");
    layerTable->setContent(11, 1, l.unitPhiOverlapLength(), coordPrecision);
    layerTable->setContent(12, 0, "horizontal overlap with (X=0) plane (each IT half) [mm]");
    layerTable->setContent(12, 1, l.installationHorizontalOverlapLength(), coordPrecision);
    layerTable->setContent(13, 0, "phiOverlap angular Ratio");
    layerTable->setContent(13, 1, l.installationOverlapRatio(), coordPrecision);
   
    tables.push_back(layerTable);
  } // end of 'fills layer table'
}
