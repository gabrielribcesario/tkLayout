#include <string>
#include <sstream>

#include "VizardTools.hh"
#include "RootWeb.hh"
#include "InnerCabling/InnerDTC.hh"
#include "InnerCabling/GBT.hh"
#include "DetectorModule.hh"
#include "AnalyzerVisitors/CMSSWInnerTrackerCablingMapVisitor.hh"

    //************************************//
    //*               Visitor             //
    //*     CMSSWInnerTrackerCablingMap   //
    //*                                   //
    //************************************//
void CMSSWInnerTrackerCablingMapVisitor::preVisit() {
  output_ << "Module_DetId/U, GBT_CMSSW_IdPerDTC/U, DTC_CMSSW_Id/U" << std::endl;
}

void CMSSWInnerTrackerCablingMapVisitor::visit(const DetectorModule& m) {
  std::stringstream moduleInfo;
  moduleInfo << m.myDetId() << ", ";

  const GBT* myGBT = m.getGBT();
  if (myGBT) {
    std::stringstream GBTInfo;
    GBTInfo << myGBT->getCMSSWId() << ",";

    const InnerDTC* myDTC = m.getInnerDTC();
    if (myDTC) {
      std::stringstream DTCInfo;
      DTCInfo << myDTC->getCMSSWId();	 
      output_ << moduleInfo.str() << GBTInfo.str() << DTCInfo.str() << std::endl;
    }
    else output_ << moduleInfo.str() << GBTInfo.str() << std::endl;
  }
  else output_ << moduleInfo.str() << std::endl;
}
