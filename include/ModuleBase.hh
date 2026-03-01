#ifndef MODULE_BASE_HH
#define MODULE_BASE_HH

#include "Property.hh"
#include "Visitable.hh"

class ModuleBase : public PropertyObject, public Buildable, public Placeable, public Identifiable<int>, public Visitable {
public:
  virtual void build() = 0;
};

class ModuleDecorable : public ModuleBase, public Decorable {};

#endif
