#ifndef ANALYZERHELPERS_HH
#define ANALYZERHELPERS_HH

#include <utility>

class DetectorModule;
class TH2D;
class Tracker;
class SimParms;

namespace AnalyzerHelpers {
  struct Point { double x, y; };
  struct Circle { double x0, y0, r; };

  std::pair<Circle, Circle> findCirclesTwoPoints(const Point& p1, const Point& p2, double r);
  bool isPointInCircle(const Point& p, const Circle& c);
  bool areClockwise(const Point& p1, const Point& p2);

  double calculatePetalAreaMC(const Tracker& tracker, const SimParms& simParms, double crossoverR);
  double calculatePetalAreaModules(const Tracker& tracker, const SimParms& simParms, double crossoverR);
  double calculatePetalCrossover(const Tracker& tracker, const SimParms& simParms);

  bool isModuleInPetal(const DetectorModule& module, double petalPhi, double curvatureR, double crossoverR);
  bool isModuleInCircleSector(const DetectorModule& module, double startPhi, double endPhi);

  bool isModuleInEtaSector(const SimParms& simParms, const Tracker& tracker, const DetectorModule& module, int etaSector); 
  bool isModuleInPhiSector(const SimParms& simParms, const DetectorModule& module, double crossoverR, int phiSector);

  void drawModuleOnMap(const DetectorModule& m, double val, TH2D& map, TH2D& counter);
  void drawModuleOnMap(const DetectorModule& m, double val, TH2D& map);
}

#endif
