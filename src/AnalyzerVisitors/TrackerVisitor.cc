#include "Barrel.hh"
#include "Endcap.hh"
#include "Layer.hh"
#include "DetectorModule.hh"
#include "AnalyzerVisitors/TrackerVisitor.hh"


    //************************************//
    //*               Visitor             //
    //*            AllModulesCsv          //
    //*                                   //
    //************************************//
void TrackerVisitor::preVisit() {
  //output_ << "Section/C:Layer/I:Ring/I:r_mm/D:z_mm/D:tiltAngle_deg/D:phi_deg/D:meanWidth_mm/D:length_mm/D:sensorSpacing_mm/D:sensorThickness_mm/D, DetId/I" << std::endl;
  output_ << "DetId/U, BinaryDetId/B, Section/C, Layer/I, Ring/I, SensorCenter rho(mm), SensorCenter z(mm), tiltAngle_deg/D, skewAngle_deg/D, yawAngle_deg/D, phi_deg/D, vtxOneX_mm/D, vtxOneY_mm/D,vtxTwoX_mm/D,vtxTwoY_mm/D,vtxThreeX_mm/D,vtxThreeY_mm/D,vtxFourX_mm/D,vtxFourY_mm/D, meanWidth_mm/D, length_mm/D, sensorSpacing_mm/D, sensorThickness_mm/D" << std::endl;
}

void TrackerVisitor::visit(const Barrel& b) {
  sectionName_ = b.myid();
}

void TrackerVisitor::visit(const Endcap& e) {
  sectionName_ = e.myid();
}

void TrackerVisitor::visit(const Layer& l) {
  layerId_ = l.myid();
}

void TrackerVisitor::visit(const Disk& d) {
  layerId_ = d.myid();
}

void TrackerVisitor::visit(const DetectorModule& m) {
  output_ << m.myDetId() << ","
	  << m.myBinaryDetId() << ","
	  << sectionName_ << ", "
	  << layerId_ << ", "
	  << m.moduleRing() << ", "
	  << std::fixed << std::setprecision(6)
	  << m.center().Rho() << ", "
	  << m.center().Z() << ", "
	  << m.tiltAngle() * 180. / M_PI << ", "
	  << m.skewAngle() * 180. / M_PI << ", "
	  << m.yawAngle() * 180. / M_PI << ", "
	  << m.center().Phi() * 180. / M_PI << ", "
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