#ifndef SENSOR_GEOMETRY_VISITOR_HH
#define SENSOR_GEOMETRY_VISITOR_HH

class Detector;
class Tracker;
class Barrel;
class Endcap;
class Layer;
class Disk;
class Ring;
class TiltedRing;
class RodPair;
class BarrelModule;
class EndcapModule;
class DetectorModule;
class Sensor;

class SensorGeometryVisitor { 
public:
  virtual ~SensorGeometryVisitor() {}
  virtual void visit(Detector&) {}
  virtual void visit(Tracker&) {}
  virtual void visit(Barrel&) {}
  virtual void visit(Endcap&) {}
  virtual void visit(Layer&) {}
  virtual void visit(Disk&) {}
  virtual void visit(Ring&) {}
  virtual void visit(TiltedRing&) {}
  virtual void visit(RodPair&) {}
  virtual void visit(BarrelModule&) {}
  virtual void visit(EndcapModule&) {}
  virtual void visit(DetectorModule&) {}
  virtual void visit(Sensor&) {}
};

#endif
