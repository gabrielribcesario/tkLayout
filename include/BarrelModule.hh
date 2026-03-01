#ifndef BARREL_MODULE_HH
#define BARREL_MODULE_HH

#include <limits>
#include <string>
#include <cmath>

#include "global_constants.hh"
#include "DetectorModule.hh"
#include "GeometryVisitor.hh"
#include "ConstGeometryVisitor.hh"
#include "SensorGeometryVisitor.hh"
#include "capabilities.hh"
#include "Property.hh"

class BarrelModule : public DetectorModule, public Clonable<BarrelModule> {
public:
  Property<int16_t, AutoDefault> layer;
  int16_t ring() const { return (int16_t)myid(); }
  int16_t moduleRing() const { return ring(); }
  Property<int16_t, AutoDefault> rod;

  BarrelModule(Decorated* decorated, const std::string subdetectorName) :
    DetectorModule(decorated, subdetectorName)
  { setup(); }

  void accept(GeometryVisitor& v) {
    v.visit(*this);
    v.visit(*(DetectorModule*)this);
    decorated().accept(v);
  }
  void accept(ConstGeometryVisitor& v) const {
    v.visit(*this);
    v.visit(*(const DetectorModule*)this);
    decorated().accept(v);
  }
  void accept(SensorGeometryVisitor& v) {
    v.visit(*this);
    v.visit(*(DetectorModule*)this);
    for (auto& s : sensors_) { s.accept(v); }
  }

  void setup() override {
    DetectorModule::setup();
    minPhi.setup([&](){
      const XYZVector& pC = basePoly().getCenter();
      double avg_phi = pC.Phi();
      auto tempPoly = basePoly();
      tempPoly.rotateZ(-avg_phi);
      // The following line will result in the module being put back in its original position
      // but with 2*M_PI more if its original average position is below -M_PI/2
      if (avg_phi < -tkLayout::PI_2) avg_phi += tkLayout::TWO_PI;
      double phi;
      double min = std::numeric_limits<double>::max();
      for (int i=0; i<4; ++i) {
        phi = tempPoly.getVertex(i).Phi();
        if (phi<min) min=phi;
      }
      min+=avg_phi;
      // Return value in interval <-pi;+3*pi> instead of <-pi;+pi> to take into account the crossline at pi/2.
      return min;
    });
    maxPhi.setup([&](){
      const XYZVector& pC = basePoly().getCenter();
      double avg_phi = pC.Phi();
      auto tempPoly = basePoly();
      tempPoly.rotateZ(-avg_phi);
      // The following line will result in the module being put back in its original position
      // but with 2*M_PI more if its original average position is below -M_PI/2
      if (avg_phi < -tkLayout::PI_2) avg_phi += tkLayout::TWO_PI;
      double max = std::numeric_limits<double>::min();
      double phi;
      for (int i=0; i<4; ++i) {
        phi = tempPoly.getVertex(i).Phi();
        if (phi>max) max=phi;
      }
      max+=avg_phi;
      // Return value in interval <-pi;+3*pi> instead of <-pi;+pi> to take into account the crossline at pi/2.
      return max;
    });

    nominalResolutionLocalX.setup([this]() {
	// only set up this if no model parameter specified
	//std::cout <<  "hasAnyResolutionLocalXParam() = " <<  hasAnyResolutionLocalXParam() << std::endl;

	if (!hasAnyResolutionLocalXParam()) {
	  //std::cout << "nominalResolutionLocalX and resolutionLocalXBarrel parameters are all unset. Use of default formulae." << std::endl;
	  double res = 0;
	  for (const Sensor& s : sensors()) res += pow(meanWidth() / s.numStripsAcross() / sqrt(12), 2);
	  return sqrt(res)/numSensors();
	}
	// if model parameters specified, return -1
	else return -1.0;
      });
    nominalResolutionLocalY.setup([this]() {
	// only set up this if no model parameters not specified
	if (!hasAnyResolutionLocalYParam()) {
	  //std::cout << "resolutionLocalY and resolutionLocalYBarrel parameters are all unset. Use of default formulae." << std::endl;
	    if (stereoRotation() != 0.) return nominalResolutionLocalX() / sin(stereoRotation());
	    else {
	      return length() / maxSegments() / sqrt(12); // NOTE: not combining measurements from both sensors. The two sensors are closer than the length of the longer sensing element, making the 2 measurements correlated. considering only the best measurement is then a reasonable approximation (since in case of a PS module the strip measurement increases the precision by only 0.2% and in case of a 2S the sensors are so close that they basically always measure the same thing)
	    }
	  }
	// if model parameters specified, return -1
	else return -1.0;
      });
  }

  void build();


  //double maxZ() const { return MAX(basePoly().getVertex(0).Z(), basePoly().getVertex(2).Z()); } 
  //double minZ() const { return MIN(basePoly().getVertex(0).Z(), basePoly().getVertex(2).Z()); } 
  //double maxR() const { return MAX(basePoly().getVertex(0).Rho(), basePoly().getVertex(2).Rho()); }
  //double minR() const { return center().Rho(); }//MIN(basePoly().getVertex(0).Rho(), basePoly().getVertex(2).Rho()); }

  virtual ModuleSubdetector subdet() const { return BARREL; }

  PosRef posRef() const { return (PosRef){ subdetectorId(), (side() > 0 ? ring() : -ring()), layer(), rod() }; }
  TableRef tableRef() const { return (TableRef){ subdetectorName(), layer(), ring() }; }
  UniRef uniRef() const { return UniRef{ subdetectorName(), layer(), ring(), rod(), side() }; }
};

#endif
