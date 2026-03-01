#ifndef OuterGBT_HH
#define OuterGBT_HH

#include <vector>
#include <string>

#include "Property.hh"
#include "DetectorModule.hh"

class OuterGBT : public PropertyObject, public Buildable, public Identifiable<int> {
  typedef std::vector<DetectorModule*> Container; 

public:
  OuterGBT(const int GBTId);

  // MODULES CONNECTED TO THE GBT
  const Container& modules() const { return modules_; }
  const int numModules() const { return modules_.size(); } 
  void addModule(DetectorModule* m);

  void setCMSSWId(const int cmsswId) { myGBTCMSSWId_ = cmsswId; }
  const int getCMSSWId() const { return myGBTCMSSWId_; }
  const int GBTId() const { return myGBTId_; }

private:
  Container modules_;

  int myGBTCMSSWId_;
  int myGBTId_;

};



#endif
