#include <cmath>
#include <utility>
#include <map>
#include <set>

#include <TRandom3.h>

#include "global_constants.hh"
#include "AnalyzerVisitors/TriggerProcessorBandwidth.hh"
#include "SimParms.hh"
#include "Barrel.hh"
#include "Tracker.hh"
#include "AnalyzerHelpers.hh"


using AnalyzerHelpers::Circle;
using AnalyzerHelpers::Point;


std::pair<Circle, Circle> AnalyzerHelpers::findCirclesTwoPoints(const Point& p1, const Point& p2, double r) {
  double x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;

  double q = sqrt(pow(x2-x1,2) + pow(y2-y1,2));
  double x3 = (x1+x2)/2;
  double y3 = (y1+y2)/2;

  double xc1 = x3 + sqrt(r*r - pow(q/2,2))*(y1-y2)/q;
  double yc1 = y3 + sqrt(r*r - pow(q/2,2))*(x2-x1)/q;

  double xc2 = x3 - sqrt(r*r - pow(q/2,2))*(y1-y2)/q;
  double yc2 = y3 - sqrt(r*r - pow(q/2,2))*(x2-x1)/q;

  return std::make_pair((Circle){ xc1, yc1, r }, (Circle){ xc2, yc2, r });
}

void TriggerProcessorBandwidthVisitor::preVisit() {
  processorConnectionSummary.setHeader("Phi", "Eta");
  processorCommonConnectionSummary.setHeader("Phi", "Eta");
  processorInboundBandwidthSummary.setHeader("Phi", "Eta");
  processorInboundStubPerEventSummary.setHeader("Phi", "Eta");

  processorInboundBandwidthSummary.setPrecision(3);
  processorInboundStubPerEventSummary.setPrecision(3);

  moduleConnectionsDistribution.Reset();
  moduleConnectionsDistribution.SetNameTitle("ModuleConnDist", "Number of connections to trigger processors;Connections;Modules");
  moduleConnectionsDistribution.SetBins(11, -.5, 10.5);


}

void TriggerProcessorBandwidthVisitor::visit(const SimParms& sp) {
  simParms_ = &sp;
  numProcEta = sp.numTriggerTowersEta();
  numProcPhi = sp.numTriggerTowersPhi();
}


void TriggerProcessorBandwidthVisitor::visit(const Tracker& t) { 
  tracker_ = &t; 
  crossoverR = AnalyzerHelpers::calculatePetalCrossover(*tracker_, *simParms_);
  sampleTriggerPetal = findCirclesTwoPoints((Point){0., 0.}, (Point){crossoverR, 0.}, simParms_->particleCurvatureR(simParms_->triggerPtCut()));
  int totalProcs = numProcEta * numProcPhi;
  processorCommonConnectionMap.SetBins(totalProcs, 0, totalProcs, totalProcs, 0, totalProcs);
  processorCommonConnectionMap.SetXTitle("TT");
  processorCommonConnectionMap.SetYTitle("TT");

}

void TriggerProcessorBandwidthVisitor::visit(const DetectorModule& m) {
  TableRef p = m.tableRef();

  uint32_t detId = m.myDetId();
  moduleConnections[&m].detId(detId);

  int etaConnections = 0, totalConnections = 0;
  for (int i=0; i < numProcEta; i++) {
    if (AnalyzerHelpers::isModuleInEtaSector(*simParms_, *tracker_, m, i)) {
      etaConnections++;
      for (int j=0; j < numProcPhi; j++) {
        if (AnalyzerHelpers::isModuleInPhiSector(*simParms_, m, crossoverR, j)) {
          totalConnections++;

          processorConnections_[std::make_pair(j,i)] += 1;
          processorConnectionSummary.setCell(j+1, i+1, processorConnections_[std::make_pair(j,i)]);

          moduleConnections[&m].connectedProcessors.insert(std::make_pair(i+1, j+1));

          processorInboundBandwidths_[std::make_pair(j,i)] += triggerDataBandwidths_[p.table][std::make_pair(p.row, p.col)]; // *2 takes into account negative Z's
          processorInboundBandwidthSummary.setCell(j+1, i+1, processorInboundBandwidths_[std::make_pair(j,i)]);

          processorInboundStubsPerEvent_[std::make_pair(j,i)] += triggerFrequenciesPerEvent_[p.table][std::make_pair(p.row, p.col)];
          processorInboundStubPerEventSummary.setCell(j+1, i+1, processorInboundStubsPerEvent_[std::make_pair(j,i)]);

          sectorMap[std::make_pair(i+1, j+1)].insert(moduleConnections[&m].detId());
        } 
      }
    }
  }
  moduleConnections[&m].etaCpuConnections(etaConnections);
  moduleConnections[&m].phiCpuConnections(totalConnections > 0 ? totalConnections/etaConnections : 0);
}

void TriggerProcessorBandwidthVisitor::postVisit() {

  for (const auto& mvp : processorInboundBandwidths_) inboundBandwidthTotal += mvp.second;
  for (const auto& mvp : processorConnections_) processorConnectionsTotal += mvp.second;
  for (const auto& mvp : processorInboundStubsPerEvent_) inboundStubsPerEventTotal += mvp.second;
  processorInboundBandwidthSummary.setSummaryCell("Total", inboundBandwidthTotal);
  processorConnectionSummary.setSummaryCell("Total", processorConnectionsTotal);
  processorInboundStubPerEventSummary.setSummaryCell("Total", inboundStubsPerEventTotal);

  std::map<std::pair<int, int>, int> processorCommonConnectionMatrix;

  for (auto mvp : moduleConnections) {
    moduleConnectionsDistribution.Fill(mvp.second.totalCpuConnections(), 1);
    std::set<std::pair<int, int>> connectedProcessors = mvp.second.connectedProcessors; // we make a copy of the set here
    if (connectedProcessors.size() == 1) {
      int ref = connectedProcessors.begin()->second + numProcPhi*(connectedProcessors.begin()->first-1);
      processorCommonConnectionMatrix[std::make_pair(ref, ref)] += 1;
    } else {
      while (!connectedProcessors.empty()) {
        std::pair<int, int> colRef = *connectedProcessors.begin();
        int col = colRef.second + numProcPhi*(colRef.first-1);
        connectedProcessors.erase(connectedProcessors.begin());
        for (std::set<std::pair<int, int> >::const_iterator pIt = connectedProcessors.begin(); pIt != connectedProcessors.end(); ++pIt) {
          int row = pIt->second + numProcPhi*(pIt->first-1);
          processorCommonConnectionMatrix[std::make_pair(row, col)] += 1;
        }
      } 
    }
  }

  TAxis* xAxis = processorCommonConnectionMap.GetXaxis();
  TAxis* yAxis = processorCommonConnectionMap.GetYaxis();
  for (int i = 1; i <= numProcEta; i++) {
    for (int j = 1; j <= numProcPhi; j++) {
      processorCommonConnectionSummary.setCell(0, j + (i-1)*numProcPhi, "t" + any2str(i) + "," + any2str(j));
      processorCommonConnectionSummary.setCell(j + (i-1)*numProcPhi, 0, "t" + any2str(i) + "," + any2str(j));
      xAxis->SetBinLabel(j + (i-1)*numProcPhi, ("t" + any2str(i) + "," + any2str(j)).c_str());
      yAxis->SetBinLabel(j + (i-1)*numProcPhi, ("t" + any2str(i) + "," + any2str(j)).c_str());
    }
  }
  for (int col = 1; col <= numProcEta*numProcPhi; col++) {
    for (int row = col; row <= numProcEta*numProcPhi; row++) {
      if (processorCommonConnectionMatrix.count(std::make_pair(row, col))) {
        int val = processorCommonConnectionMatrix[std::make_pair(row, col)];
        processorCommonConnectionSummary.setCell(row, col, val);
        processorCommonConnectionMap.SetCellContent(row, col, val/2);
        if (row != col) processorCommonConnectionMap.SetCellContent(col, row, val/2);
        else processorCommonConnectionMap.SetCellContent(row, col, val);
      }
      //else processorCommonConnectionSummary_.setCell(row, col, "0");
    }
  }
}

