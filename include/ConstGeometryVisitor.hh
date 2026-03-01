#ifndef CONSTGEOMETRYVISITOR_HH
#define CONSTGEOMETRYVISITOR_HH

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
class RectangularModule;
class WedgeModule;
class GeometricModule;
class SimParms;

class ConstGeometryVisitor {
public:
  virtual ~ConstGeometryVisitor() {}
  virtual void visit(const Detector&) {}
  virtual void visit(const Tracker&) {}
  virtual void visit(const Barrel&) {}
  virtual void visit(const Endcap&) {}
  virtual void visit(const Layer&) {}
  virtual void visit(const Disk&) {}
  virtual void visit(const TiltedRing&) {}
  virtual void visit(const Ring&) {}
  virtual void visit(const RodPair&) {}
  virtual void visit(const BarrelModule&) {}
  virtual void visit(const EndcapModule&) {}
  virtual void visit(const DetectorModule&) {}
  virtual void visit(const RectangularModule&) {}
  virtual void visit(const WedgeModule&) {}
  virtual void visit(const GeometricModule&) {}
  virtual void visit(const SimParms&) {}
};

#endif
