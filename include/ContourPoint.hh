#ifndef CONTOURPOINT_H
#define CONTOURPOINT_H

#include "capabilities.hh"
#include "global_funcs.hh"
#include "Property.hh"

class ContourPoint : public PropertyObject, public Buildable, public Identifiable<int>, public DetIdentifiable {
 public:
  ReadonlyProperty<double, NoDefault> pointX;
  ReadonlyProperty<double, NoDefault> pointY;

 ContourPoint() :
  pointX("pointX", parsedOnly()),
    pointY("pointY", parsedOnly())
      {}

};

#endif
