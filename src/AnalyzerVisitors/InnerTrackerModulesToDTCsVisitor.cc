#include <string>
#include <sstream>

#include "VizardTools.hh"
#include "RootWeb.hh"
#include "Barrel.hh"
#include "Endcap.hh"
#include "Layer.hh"
#include "Disk.hh"
#include "DetectorModule.hh"
#include "InnerCabling/InnerDTC.hh"
#include "InnerCabling/InnerBundle.hh"
#include "InnerCabling/PowerChain.hh"
#include "InnerCabling/GBT.hh"
#include "AnalyzerVisitors/InnerTrackerModulesToDTCsVisitor.hh"

    //************************************//
    //*               Visitor             //
    //*    InnerTrackerModulesToDTCsCsv   //
    //*                                   //
    //************************************//

void InnerTrackerModulesToDTCsVisitor::preVisit() {
  output_ << "Module_DetId/i, Module_SubType/I, Module_Section/C, Module_Layer/I, Module_Ring/I, Module_phi_deg/D, N_Chips_Per_Module/I, N_Channels_Per_Module/I, Is_LongBarrel/O, Power_Chain/I, Power_Chain_Type/C, N_ELinks_Per_Module/I, LpGBT_Id/C, LpGBT_CMSSW_IdPerDTC/U, MFB/I, DTC_Id/I, DTC_CMSSW_Id/U, IsPlusZEnd/O, IsPlusXSide/O" << std::endl;
}

void InnerTrackerModulesToDTCsVisitor::visit(const Barrel& b) {
  sectionName_ = b.myid();
}

void InnerTrackerModulesToDTCsVisitor::visit(const Endcap& e) {
  sectionName_ = e.myid();
}

void InnerTrackerModulesToDTCsVisitor::visit(const Layer& l) {
  layerId_ = l.myid();
}

void InnerTrackerModulesToDTCsVisitor::visit(const Disk& d) {
  layerId_ = d.myid();
}

void InnerTrackerModulesToDTCsVisitor::visit(const DetectorModule& m) {
  const PowerChain* myPowerChain = m.getPowerChain();
  if (myPowerChain != nullptr) {
    std::stringstream moduleInfo;
    moduleInfo << m.myDetId() << ","
         << m.moduleSubType() << ","
	       << sectionName_ << ", "
	       << layerId_ << ", "
	       << m.moduleRing() << ", "
	       << std::fixed << std::setprecision(6)
	       << m.center().Phi() * 180. / M_PI << ", "
	       << m.outerSensor().totalROCs() << ", "
	       << m.totalChannels() << ", ";

    std::stringstream powerChainInfo;
    powerChainInfo << any2str(myPowerChain->isLongBarrel()) << ","
		   << myPowerChain->myid() << ","
		   << any2str(myPowerChain->powerChainType()) << ",";

    const GBT* myGBT = m.getGBT();
    if (myGBT != nullptr) {
      std::stringstream GBTInfo;
      GBTInfo << myGBT->numELinksPerModule() << ","
	      << any2str(myGBT->GBTId()) << ","
	      << myGBT->getCMSSWId() << ",";

      const InnerBundle* myBundle = myGBT->getBundle();
      if (myBundle != nullptr) {
	std::stringstream bundleInfo;
	bundleInfo << myBundle->myid() << ",";
	
	const InnerDTC* myDTC = myBundle->getDTC();
	if (myDTC != nullptr) {
	  std::stringstream DTCInfo;
	  DTCInfo << myDTC->myid() << ","
		  << myDTC->getCMSSWId() << ","
		  << myDTC->isPositiveZEnd() << ","
		  << myDTC->isPositiveXSide();
	  output_ << moduleInfo.str() << powerChainInfo.str() << GBTInfo.str() << bundleInfo.str() << DTCInfo.str() << std::endl;
	}
	else output_ << moduleInfo.str() << powerChainInfo.str() << GBTInfo.str() << bundleInfo.str() << std::endl;
      }
      else output_ << moduleInfo.str() << powerChainInfo.str() << GBTInfo.str() << std::endl;
    }
    else output_ << moduleInfo.str() << powerChainInfo.str() << std::endl;
  }
}