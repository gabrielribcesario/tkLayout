#include <string>

#include "global_constants.hh"
#include "Layer.hh"
#include "BarrelModule.hh"
#include "Barrel.hh"
#include "AnalyzerVisitors/BarrelVisitor.hh"


    //************************************//
    //*               Visitor             //
    //*            BarrelModulesCsv       //
    //*                                   //
    //************************************//
void BarrelVisitor::preVisit() {
  output_ << "DetId, BinaryDetId, Barrel-Layer name, SensorCenter rho(mm), SensorCenter z(mm), tiltAngle(deg), num mods, meanWidth(mm) (orthoradial), length(mm) (along Z), sensorSpacing(mm), sensorThickness(mm)" << std::endl;
}
void BarrelVisitor::visit(const Barrel& b) {
  barName_ = b.myid();
}
void BarrelVisitor::visit(const Layer& l) {
  layId_ = l.myid();
  numRods_ = l.numRods();
}
void BarrelVisitor::visit(const BarrelModule& m) {
  if (m.posRef().phi > 2) return;
  output_ << m.myDetId() << ", "
	  << m.myBinaryDetId() << ","
	  << barName_ << "-L" << layId_ << ", "
	  << std::fixed << std::setprecision(6)
	  << m.center().Rho() << ", "
	  << m.center().Z() << ", "
	  << m.tiltAngle() * tkLayout::RAD_TO_DEG << ", "
	  << numRods_/2. << ", "
	  << m.meanWidth() << ", "
	  << m.length() << ", "
	  << m.dsDistance() << ", "
	  << m.sensorThickness()
	  << std::endl;
}

std::string BarrelVisitor::output() const { return output_.str(); }
