#include <string>

#include "global_constants.hh"
#include "Barrel.hh"
#include "Endcap.hh"
#include "Layer.hh"
#include "Disk.hh"
#include "Sensor.hh"
#include "DetectorModule.hh"
#include "TrackerSensorVisitor.hh"

    //************************************//
    //*               Visitor             //
    //*            Sensors DetIds         //
    //*                                   //
    //************************************//
void TrackerSensorVisitor::visit(Barrel& b) {
  sectionName_ = b.myid();
}

void TrackerSensorVisitor::visit(Endcap& e) {
  sectionName_ = e.myid();
}

void TrackerSensorVisitor::visit(Layer& l)  {
  layerId_ = l.myid();
}

void TrackerSensorVisitor::visit(Disk& d)  {
  layerId_ = d.myid();
}

void TrackerSensorVisitor::visit(DetectorModule& m)  {
  moduleRing_ = m.moduleRing();
}

void TrackerSensorVisitor::visit(Sensor& s) {
  output_ << s.myDetId() << ","
	  << s.myBinaryDetId() << ","
	  << sectionName_ << ", "
	  << layerId_ << ", "
	  << moduleRing_ << ", "
	  << std::fixed << std::setprecision(6)
	  << s.hitPoly().getCenter().Rho() << ", "
	  << s.hitPoly().getCenter().Z() << ", "
	  << s.hitPoly().getCenter().Phi() * tkLayout::RAD_TO_DEG
	  << std::endl;
}

std::string TrackerSensorVisitor::output() const { return output_.str(); }
