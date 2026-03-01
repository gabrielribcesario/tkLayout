#ifndef CMSSWOUTERTRACKERCABLINGMAPVISITOR_HH
#define	CMSSWOUTERTRACKERCABLINGMAPVISITOR_HH

#include <string>
#include <sstream>

#include "ConstGeometryVisitor.hh"

class DetectorModule;

    //************************************//
    //*               Visitor             //
    //*     CMSSWOuterTrackerCablingMap   //
    //*                                   //
    //************************************//
class CMSSWOuterTrackerCablingMapVisitor : public ConstGeometryVisitor {
  std::stringstream output_;
 
public:
  void preVisit();
  void visit(const DetectorModule& m);
  std::string output() const { return output_.str(); }
};

#endif
