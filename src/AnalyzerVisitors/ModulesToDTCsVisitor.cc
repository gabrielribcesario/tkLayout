#include <string>
#include <sstream>

#include "global_constants.hh"
#include "Barrel.hh"
#include "Endcap.hh"
#include "Layer.hh"
#include "Disk.hh"
#include "DetectorModule.hh"
#include "OuterCabling/OuterCable.hh"
#include "OuterCabling/OuterDTC.hh"
#include "OuterCabling/OuterBundle.hh"
#include "AnalyzerVisitors/ModulesToDTCsVisitor.hh" 

    //************************************//
    //*               Visitor             //
    //*            ModulesToDTCsCsv       //
    //*                                   //
    //************************************//
ModulesToDTCsVisitor::ModulesToDTCsVisitor(bool isPositiveCablingSide) {
  isPositiveCablingSide_ = isPositiveCablingSide;
}

void ModulesToDTCsVisitor::preVisit() {
  output_ << "Module_DetId/i, Module_Section/C, Module_Layer/I, Module_Ring/I, Module_phi_deg/D, MFB/I, OPT_Services_Channel/I, PWR_Services_Channel/I, MFC/I, MFC_type/C, DTC_name/C, DTC_CMSSW_Id/i, DTC_Phi_Sector_Ref/I, type_/C, DTC_Slot/I, DTC_Phi_Sector_Width_deg/D" << std::endl;
}

void ModulesToDTCsVisitor::visit(const Barrel& b) {
  sectionName_ = b.myid();
}

void ModulesToDTCsVisitor::visit(const Endcap& e) {
  sectionName_ = e.myid();
}

void ModulesToDTCsVisitor::visit(const Layer& l) {
  layerId_ = l.myid();
}

void ModulesToDTCsVisitor::visit(const Disk& d) {
  layerId_ = d.myid();
}

void ModulesToDTCsVisitor::visit(const DetectorModule& m) {
  const OuterBundle* myBundle = m.getBundle();
  if (myBundle != nullptr) {
    if (myBundle->isPositiveCablingSide() == isPositiveCablingSide_) {
      std::stringstream moduleInfo;
      moduleInfo << m.myDetId() << ", "
		 << sectionName_ << ", "
		 << layerId_ << ", "
		 << m.moduleRing() << ", "
		 << std::fixed << std::setprecision(6)
		 << m.center().Phi() * tkLayout::RAD_TO_DEG << ", ";

      std::stringstream bundleInfo;
      bundleInfo << myBundle->myid() << ", ";

      const OuterCable* myCable = myBundle->getCable();
      if (myCable != nullptr) {
	std::stringstream cableInfo;
	cableInfo << myCable->myid() << ", "
		  << any2str(myCable->type()) << ", ";
	bundleInfo << myCable->opticalChannelSection()->channelNumber() << " " 
		   << any2str(myCable->opticalChannelSection()->channelSlot()) << ", "
		   << myBundle->powerChannelSection()->channelNumber() << " " 
		   << any2str(myBundle->powerChannelSection()->channelSlot()) << ", ";
	
	const OuterDTC* myDTC = myCable->getDTC();
	if (myDTC != nullptr) {
	  std::stringstream DTCInfo;
	  DTCInfo << myDTC->name() << ", "
		  << myDTC->getCMSSWId() << ", "
		  << myDTC->phiSectorRef() << ", "
		  << any2str(myDTC->type()) << ", "
		  << myDTC->slot() << ", "
		  << std::fixed << std::setprecision(6)
		  << myDTC->phiSectorWidth() * tkLayout::RAD_TO_DEG;
	  output_ << moduleInfo.str() << bundleInfo.str() << cableInfo.str() << DTCInfo.str() << std::endl;
	}
	else output_ << moduleInfo.str() << bundleInfo.str() << cableInfo.str() << std::endl;
      }
      else output_ << moduleInfo.str() << bundleInfo.str() << std::endl;
    }
  }
}
