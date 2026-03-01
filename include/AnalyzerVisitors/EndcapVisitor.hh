#ifndef ENDCAPVISITOR_HH
#define	ENDCAPVISITOR_HH

#include <string>
#include <sstream>

#include "ConstGeometryVisitor.hh"

    //************************************//
    //*               Visitor             //
    //*            EndcapModulesCsv       //
    //*                                   //
    //************************************//
class EndcapVisitor : public ConstGeometryVisitor {
  std::stringstream output_;
  std::string endcapName_;
  int diskId_;

public:
  void preVisit();
  void visit(const Endcap& e);
  void visit(const Disk& d);
  void visit(const EndcapModule& m);

  std::string output() const;
};

#endif
