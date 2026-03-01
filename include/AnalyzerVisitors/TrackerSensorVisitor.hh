#ifndef TRACKERSENSORVISITOR_HH
#define	TRACKERSENSORVISITOR_HH

#include <string>
#include <sstream>

#include "SensorGeometryVisitor.hh"

class Barrel;
class Endcap;
class Layer;
class Disk;
class DetectorModule;
class Sensor;

    //************************************//
    //*               Visitor             //
    //*            Sensors DetIds         //
    //*                                   //
    //************************************//
class TrackerSensorVisitor : public SensorGeometryVisitor {
  std::stringstream output_;
  std::string sectionName_;
  int layerId_;
  int moduleRing_;

public:
  void visit(Barrel& b);
  void visit(Endcap& e);
  void visit(Layer& l);
  void visit(Disk& d);
  void visit(DetectorModule& m);
  void visit(Sensor& s);

  std::string output() const;
};

#endif
