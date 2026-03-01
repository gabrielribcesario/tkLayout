#ifndef VISITABLE_HH
#define VISITABLE_HH

class GeometryVisitor;
class ConstGeometryVisitor;

class Visitable {
public:
  Visitable(){};
  virtual ~Visitable(){};

  virtual void accept(GeometryVisitor& v) = 0;
  virtual void accept(ConstGeometryVisitor& v) const = 0;
};

#endif
