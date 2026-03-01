#include <string>

#include "global_constants.hh"
#include "Endcap.hh"
#include "Disk.hh"
#include "EndcapModule.hh"
#include "AnalyzerVisitors/EndcapVisitor.hh"

    //************************************//
    //*               Visitor             //
    //*            EndcapModulesCsv       //
    //*                                   //
    //************************************//
void EndcapVisitor::preVisit() {
  output_ << "DetId, BinaryDetId, Endcap-Disc name, Ring, SensorCenter rho(mm), SensorCenter z(mm), tiltAngle(deg), yawAngle(deg), phi(deg), vtxOneX_mm/D, vtxOneY_mm/D,vtxTwoX_mm/D,vtxTwoY_mm/D,vtxThreeX_mm/D,vtxThreeY_mm/D,vtxFourX_mm/D,vtxFourY_mm/D, meanWidth(mm) (orthoradial), length(mm) (radial), sensorSpacing(mm), sensorThickness(mm)" << std::endl;
}

void EndcapVisitor::visit(const Endcap& e) {
  endcapName_ = e.myid();
}

void EndcapVisitor::visit(const Disk& d)  {
  diskId_ = d.myid();
}

void EndcapVisitor::visit(const EndcapModule& m) {
  if (m.minZ() < 0.) return;

  output_	<< m.myDetId() << ", "
		<< m.myBinaryDetId() << ","
		<< endcapName_ << "-D" << diskId_ << ", "
		<< m.ring() << ", "
		<< std::fixed << std::setprecision(6)
		<< m.center().Rho() << ", "
		<< m.center().Z() << ", "
		<< m.tiltAngle() * tkLayout::RAD_TO_DEG << ", "
		<< m.yawAngle() * tkLayout::RAD_TO_DEG << ", "
		<< m.center().Phi() * tkLayout::RAD_TO_DEG << ", "
                << m.getVertex(0).X() << ", "
                << m.getVertex(0).Y() << ", "
                << m.getVertex(1).X() << ", "
                << m.getVertex(1).Y() << ", "
                << m.getVertex(2).X() << ", "
                << m.getVertex(2).Y() << ", "
                << m.getVertex(3).X() << ", "
                << m.getVertex(3).Y() << ", "
		<< m.meanWidth() << ", "
		<< m.length() << ", "
		<< m.dsDistance() << ", "
		<< m.sensorThickness()
		<< std::endl;
}

std::string EndcapVisitor::output() const { return output_.str(); }
