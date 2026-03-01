#ifndef SKEWEDLAYERSVISITOR_HH
#define	SKEWEDLAYERSVISITOR_HH

#include <vector>

#include "global_constants.hh"
#include "ConstGeometryVisitor.hh"

class Layer;
class RootWTable;

    //***************************************//
    //*                Visitor              *//
    //*             Skewed layers:          *//
    //*            Additional info          *//
    //*                                     *//
    //***************************************//

class SkewedLayersVisitor : public ConstGeometryVisitor {
public:
  std::vector<RootWTable*> tables;

  // counter
  int numSkewedLayers = 0;

  void visit(const Layer& l) override;     
};

#endif
