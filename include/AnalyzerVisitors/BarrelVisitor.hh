#ifndef BARRELVISITOR_HH
#define	BARRELVISITOR_HH

#include <string>
#include <sstream>

#include "ConstGeometryVisitor.hh"

class Barrel;
class Layer;
class DetectorModule;

    //************************************//
    //*               Visitor             //
    //*            BarrelModulesCsv       //
    //*                                   //
    //************************************//
class BarrelVisitor : public ConstGeometryVisitor {
  std::stringstream output_;
  std::string barName_;
  int layId_;
  int numRods_;

public:
  void preVisit();
  void visit(const Barrel& b);
  void visit(const Layer& l);
  void visit(const BarrelModule& m);
  std::string output() const;
};

#endif
