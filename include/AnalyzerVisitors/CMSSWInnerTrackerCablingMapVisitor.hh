#ifndef CMSSWINNERTRACKERCABLINGMAPVISITOR_HH
#define	CMSSWINNERTRACKERCABLINGMAPVISITOR_HH

#include <string>
#include <sstream>

#include "ConstGeometryVisitor.hh"

class DetectorModule;

    //************************************//
    //*               Visitor             //
    //*     CMSSWInnerTrackerCablingMap   //
    //*                                   //
    //************************************//
class CMSSWInnerTrackerCablingMapVisitor : public ConstGeometryVisitor {
  std::stringstream output_;
 
public:
  void preVisit();
  void visit(const DetectorModule& m);
  std::string output() const { return output_.str(); }
};

#endif
