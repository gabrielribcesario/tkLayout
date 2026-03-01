#ifndef MATERIALBILLANALYZER_HH
#define MATERIALBILLANALYZER_HH

#include <string>
#include <map>
#include <vector>

#include "Tracker.hh"
#include "InactiveElement.hh"

class InactiveElement;
namespace insur { class MaterialBudget; }

namespace MaterialBillAnalyzerData {
  typedef std::map<std::string, double> MaterialMap;

  class ServiceElement {
  public: 
    double zmin, zmax, rmin, rmax;
    MaterialMap materialMap;
  };
}

class MaterialBillAnalyzer {
 private:
  typedef std::map<std::string, MaterialBillAnalyzerData::MaterialMap> LayerMaterialMap;
  typedef std::vector<MaterialBillAnalyzerData::ServiceElement> ServicesMaterialVector;
  ServicesMaterialVector servicesMaterialVector_;
  LayerMaterialMap layerMaterialMap_;
  void inspectInactiveElements(const std::vector<InactiveElement>& inactiveElements);
  void inspectModules(std::vector<std::vector<insur::ModuleCap> >& tracker);

 public:
  std::string outputTable;
  void inspectTracker(insur::MaterialBudget&);
};

#endif
