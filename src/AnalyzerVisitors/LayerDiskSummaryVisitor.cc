#include <string>

#include "VizardTools.hh"
#include "RootWeb.hh"
#include "SimParms.hh"
#include "Endcap.hh"
#include "Layer.hh"
#include "TagMaker.hh"
#include "DetectorModule.hh"
#include "Ring.hh"
#include "Barrel.hh"
#include "Disk.hh"
#include "SupportStructure.hh"
#include "AnalyzerVisitors/LayerDiskSummaryVisitor.hh"


   //**************************************//
    //*             Visitor                *//
    //*  Layers and disks : global info    *//
    //*                                    *//
    //**************************************//

void LayerDiskSummaryVisitor::preVisit() {
  layerTable->setContent(0, 0, "Barrel :");
  layerTable->setContent(1, 0, "Layer");
  layerTable->setContent(2, 0, "r");
  layerTable->setContent(3, 0, "z_max");
  layerTable->setContent(4, 0, "# rods");
  layerTable->setContent(5, 0, "# mods");
  diskTable->setContent(0, 0, "Endcap :");
  diskTable->setContent(1, 0, "Disk");
  diskTable->setContent(2, 0, "z");
  diskTable->setContent(3, 0, "# rings");
  diskTable->setContent(4, 0, "# mods");
  nMB = SimParms::getInstance().numMinBiasEvents();
}

void LayerDiskSummaryVisitor::visit(const Barrel& b) {
  layerTable->setContent(0, 1 + nBarrelLayers, b.myid());
}

void LayerDiskSummaryVisitor::visit(const Layer& l) {
  if (l.maxZ() < 0.) return;
  ++nBarrelLayers;
  totalBarrelModules += l.totalModules();
  layerTable->setContent(1, nBarrelLayers, l.myid());
  layerTable->setContent(2, nBarrelLayers, l.placeRadius(), coordPrecision);
  layerTable->setContent(3, nBarrelLayers, l.maxZ(), coordPrecision);
  layerTable->setContent(4, nBarrelLayers, l.numRods());
  layerTable->setContent(5, nBarrelLayers, l.totalModules());
}

void LayerDiskSummaryVisitor::visit(const Endcap& e) {
  nEndcaps++;
  endcapId = e.myid();
  diskTable->setContent(0, 1 + nDisks, endcapId);

  RootWTable* endcapName = new RootWTable();
  endcapName->setContent(0, 0, endcapId + ",  Disc 1 :");
  endcapNames.push_back(endcapName);

  RootWTable* endcapTable = new RootWTable();
  endcapTable->setContent(0, 0, "Ring :");
  endcapTable->setContent(1, 0, "r"+subStart+"min"+subEnd);
  endcapTable->setContent(2, 0, "r"+subStart+"low"+subEnd);
  endcapTable->setContent(3, 0, "r"+subStart+"centre"+subEnd);
  endcapTable->setContent(4, 0, "r"+subStart+"high"+subEnd);
  endcapTable->setContent(5, 0, "r"+subStart+"max"+subEnd);
  endcapTable->setContent(6, 0, "phiOverlap");
  endcapTable->setContent(7, 0, "# mods");
  endcapTables.push_back(endcapTable);
}

void LayerDiskSummaryVisitor::visit(const Disk& d) {
  nRings = 0;
  nRingsTotal = d.numRings() - d.numEmptyRings();
  if (d.centerZ() < 0.) return;
  ++nDisks;
  totalEndcapModules += d.totalModules();
  diskTable->setContent(1, nDisks, d.myid());
  diskTable->setContent(2, nDisks, d.centerZ(), coordPrecision);
  diskTable->setContent(4, nDisks, d.totalModules());

  RootWTable* diskName = new RootWTable();
  diskId = endcapId + ",  Disc " + std::to_string(d.myid()) + " :";
  diskName->setContent(0, 0, diskId);
  diskNames.push_back(diskName);

  RootWTable* zErrorTable = new RootWTable();
  zErrorTable->setContent(0, 0, "Ring :");
  zErrorTable->setContent(1, 0, "zError (Ring i & i+1)");
  zErrorTables.push_back(zErrorTable);
}

void LayerDiskSummaryVisitor::visit(const Ring& r) {
  if (r.averageZ() < 0. || r.numModules() == 0) return;
  ++nRings;
  diskTable->setContent(3, nDisks, nRings);
  endcapTables.at(nEndcaps-1)->setContent(0, nRings, r.myid());
  endcapTables.at(nEndcaps-1)->setContent(6, nRings, r.actualPhiOverlap(), coordPrecision);
  endcapTables.at(nEndcaps-1)->setContent(7, nRings, r.numModules());
  zErrorTables.at(nDisks-1)->setContent(0, nRings, r.myid());
  if (nRings != nRingsTotal) zErrorTables.at(nDisks-1)->setContent(1, nRings, r.actualZError(), coordPrecision);
}

void LayerDiskSummaryVisitor::visit(const DetectorModule& m) {
  TagMaker tmak(m);

  std::string aSensorTag = tmak.sensorGeoTag;
  tagMapPositions[aSensorTag].insert(tmak.posTag);
  tagMapCount[aSensorTag]++;
  tagMapCountChan[aSensorTag] += m.totalChannels();
  tagMapMaxStripOccupancy[aSensorTag] = MAX(m.stripOccupancyPerEvent()*nMB, tagMapMaxStripOccupancy[aSensorTag]);
  tagMapMaxHitOccupancy[aSensorTag] = MAX(m.hitOccupancyPerEvent()*nMB, tagMapMaxHitOccupancy[aSensorTag]);
  tagMapAveStripOccupancy[aSensorTag] += m.stripOccupancyPerEvent()*nMB;
  tagMapAveHitOccupancy[aSensorTag] += m.hitOccupancyPerEvent()*nMB;
  // modules' spatial resolution along the local X axis is not parametrized
  if (!m.hasAnyResolutionLocalXParam()) {
    tagMapIsParametrizedXResolution[aSensorTag] = false;
    tagMapAveRphiResolution[aSensorTag] += m.nominalResolutionLocalX();
    tagMapAveRphiResolutionRmse[aSensorTag] += 0.;
  }
  // modules' spatial resolution along the local X axis is parametrized
  else {
    tagMapIsParametrizedXResolution[aSensorTag] = true;
    if (boost::accumulators::count(m.rollingParametrizedResolutionLocalX) > 0) { // calculation only on hit modules
      tagMapSumXResolution[aSensorTag] += sum(m.rollingParametrizedResolutionLocalX);
      tagMapSumSquaresXResolution[aSensorTag] += moment<2>(m.rollingParametrizedResolutionLocalX) * boost::accumulators::count(m.rollingParametrizedResolutionLocalX);
      tagMapCountXResolution[aSensorTag] += double(boost::accumulators::count(m.rollingParametrizedResolutionLocalX));
    }
  }
  // modules' spatial resolution along the local Y axis is not parametrized
  if (!m.hasAnyResolutionLocalYParam()) {
    tagMapIsParametrizedYResolution[aSensorTag] = false;
    tagMapAveYResolution[aSensorTag] += m.nominalResolutionLocalY();
    tagMapAveYResolutionRmse[aSensorTag] += 0.;
  }
  // modules' spatial resolution along the local Y axis is parametrized
  else {
    tagMapIsParametrizedYResolution[aSensorTag] = true;
    if (boost::accumulators::count(m.rollingParametrizedResolutionLocalY) > 0) { // calculation only on hit modules
      tagMapSumYResolution[aSensorTag] += sum(m.rollingParametrizedResolutionLocalY);
      tagMapSumSquaresYResolution[aSensorTag] += moment<2>(m.rollingParametrizedResolutionLocalY) * boost::accumulators::count(m.rollingParametrizedResolutionLocalY);
      tagMapCountYResolution[aSensorTag] += double(boost::accumulators::count(m.rollingParametrizedResolutionLocalY));
    }
  }
  //tagMapAveRphiResolutionTrigger[aSensorTag] += m.resolutionRPhiTrigger();
  //tagMapAveYResolutionTrigger[aSensorTag] += m.resolutionYTrigger();
  tagMapSensorPowerAvg[aSensorTag] += m.sensorsIrradiationPowerMean();
  if (tagMapSensorPowerMax[aSensorTag] < m.sensorsIrradiationPowerMean()) tagMapSensorPowerMax[aSensorTag] = m.sensorsIrradiationPowerMean();
  totCountMod++;
  totCountSens += m.numSensors();
  totChannel += m.totalChannels();
  totalSensorPower += m.sensorsIrradiationPowerMean();
  if (tagMap.find(aSensorTag)==tagMap.end()){
    // We have a new sensor geometry
    tagMap[aSensorTag] = &m;
  }
  // Summing sensor areas calculated from their respective polygons
  for (int i = 0; i < m.numSensors(); i++) {
    totArea += m.sensors().at(i).hitPoly().getDoubleArea()/2.;
  }
}

void LayerDiskSummaryVisitor::visit(const EndcapModule& m) {
  if (m.side() != 1 || m.disk() != 1) return;

  endcapTables.at(nEndcaps-1)->setContent(1, nRings, m.minR(), coordPrecision);
  endcapTables.at(nEndcaps-1)->setContent(2, nRings, sqrt(pow(m.minR(),2)+pow(m.minWidth()/2.,2)), coordPrecision); // Ugly, this should be accessible as a method
  endcapTables.at(nEndcaps-1)->setContent(3, nRings, m.center().Rho(), coordPrecision);
  endcapTables.at(nEndcaps-1)->setContent(4, nRings, m.minR()+m.length(), coordPrecision);
  endcapTables.at(nEndcaps-1)->setContent(5, nRings, m.maxR(), coordPrecision);
}

void LayerDiskSummaryVisitor::postVisit() {
  layerTable->setContent(0, nBarrelLayers+1, "Total");
  layerTable->setContent(5, nBarrelLayers+1, totalBarrelModules);
  diskTable->setContent(0, nDisks+1, "Total");
  diskTable->setContent(4, nDisks+1, totalEndcapModules*2);
}
