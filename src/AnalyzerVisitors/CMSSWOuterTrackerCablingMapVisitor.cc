#include <sstream>

#include "DetectorModule.hh"
#include "OuterCabling/OuterGBT.hh"
#include "OuterCabling/OuterDTC.hh"
#include "AnalyzerVisitors/CMSSWOuterTrackerCablingMapVisitor.hh"

    //************************************//
    //*               Visitor             //
    //*     CMSSWOuterTrackerCablingMap   //
    //*                                   //
    //************************************//
void CMSSWOuterTrackerCablingMapVisitor::preVisit() {
  output_ << "Module_DetId/U, GBT_CMSSW_IdPerDTC/U, DTC_CMSSW_Id/U" << std::endl;
}

void CMSSWOuterTrackerCablingMapVisitor::visit(const DetectorModule& m) {
  std::stringstream moduleInfo;
  moduleInfo << m.myDetId() << ", ";
  const OuterGBT* myGBT = m.getOuterGBT();
  if (myGBT) {
    std::stringstream GBTInfo;
    GBTInfo << myGBT->getCMSSWId() << ", ";
    const OuterDTC* myDTC = m.getDTC();
    if (myDTC) {
      std::stringstream DTCInfo;
      DTCInfo << myDTC->getCMSSWId();	 
      output_ << moduleInfo.str() << GBTInfo.str() << DTCInfo.str() << std::endl;
    }
  }
  else output_ << moduleInfo.str() << std::endl;
}