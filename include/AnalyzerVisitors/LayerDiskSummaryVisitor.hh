#ifndef LAYERDISKSUMMARYVISITOR_HH
#define	LAYERDISKSUMMARYVISITOR_HH

#include <string>
#include <vector>
#include <map>
#include <set>

#include "ConstGeometryVisitor.hh"

class RootWTable;
class DetectorModule;
class Endcap;
class EndcapModule;
class Barrel;
class Layer;
class Disk;
class Ring;

    //**************************************//
    //*             Visitor                *//
    //*  Layers and disks : global info    *//
    //*                                    *//
    //**************************************//

    // Build the module type maps
    // with a pointer to a sample module
class LayerDiskSummaryVisitor : public ConstGeometryVisitor {
public:
  // info
  RootWTable* layerTable = new RootWTable();  
  RootWTable* diskTable = new RootWTable();
  std::vector<RootWTable*> endcapNames;
  std::vector<RootWTable*> endcapTables;
  std::vector<RootWTable*> diskNames;
  std::vector<RootWTable*> zErrorTables;
  std::map<std::string, std::set<std::string> > tagMapPositions;
  std::map<std::string, int> tagMapCount;
  std::map<std::string, long> tagMapCountChan;
  std::map<std::string, double> tagMapMaxStripOccupancy;
  std::map<std::string, double> tagMapAveStripOccupancy;
  std::map<std::string, double> tagMapMaxHitOccupancy;
  std::map<std::string, double> tagMapAveHitOccupancy;
  std::map<std::string, double> tagMapAveRphiResolution;
  std::map<std::string, double> tagMapAveRphiResolutionRmse;
  std::map<std::string, double> tagMapSumXResolution;
  std::map<std::string, double> tagMapSumSquaresXResolution;
  std::map<std::string, double> tagMapCountXResolution;
  std::map<std::string, double> tagMapIsParametrizedXResolution;
  std::map<std::string, double> tagMapAveYResolution;
  std::map<std::string, double> tagMapAveYResolutionRmse;
  std::map<std::string, double> tagMapSumYResolution;
  std::map<std::string, double> tagMapSumSquaresYResolution;
  std::map<std::string, double> tagMapCountYResolution;
  std::map<std::string, double> tagMapIsParametrizedYResolution;
  std::map<std::string, double> tagMapAveRphiResolutionTrigger;
  std::map<std::string, double> tagMapAveYResolutionTrigger;
  std::map<std::string, double> tagMapSensorPowerAvg;
  std::map<std::string, double> tagMapSensorPowerMax;
  std::map<std::string, const DetectorModule*> tagMap;

  std::string endcapId;
  std::string diskId;

  // counters
  int nBarrelLayers = 0;
  int nEndcaps = 0;
  int nDisks = 0;
  int nRings = 0;
  int nRingsTotal = 0;
  int totalBarrelModules = 0;
  int totalEndcapModules = 0;

  double totArea = 0;
  int totCountMod = 0;
  int totCountSens = 0;
  long totChannel = 0;
  double totalSensorPower = 0;

  double nMB;

  void preVisit();
  void visit(const Barrel& b) override;
  void visit(const Layer& l) override;
  void visit(const Endcap& e) override;
  void visit(const Disk& d) override;
  void visit(const Ring& r) override;
  void visit(const DetectorModule& m) override;
  void visit(const EndcapModule& m) override;
  void postVisit();
};

#endif
