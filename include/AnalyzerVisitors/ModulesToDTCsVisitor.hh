#ifndef MODULESTODTCSVISITOR_HH
#define	MODULESTODTCSVISITOR_HH

#include <string>
#include <sstream>

#include "ConstGeometryVisitor.hh"

class Barrel;
class Endcap;
class Layer;
class Disk;
class DetectorModule;

    //************************************//
    //*               Visitor             //
    //*            ModulesToDTCsCsv       //
    //*                                   //
    //************************************//
class ModulesToDTCsVisitor : public ConstGeometryVisitor {
  bool isPositiveCablingSide_;
  std::stringstream output_;
  std::string sectionName_;
  int layerId_;

public:
  ModulesToDTCsVisitor(bool isPositiveCablingSide);
  void preVisit();
  void visit(const Barrel& b);
  void visit(const Endcap& e);
  void visit(const Layer& l);
  void visit(const Disk& d);
  void visit(const DetectorModule& m);
  std::string output() const { return output_.str(); }
};

#endif
