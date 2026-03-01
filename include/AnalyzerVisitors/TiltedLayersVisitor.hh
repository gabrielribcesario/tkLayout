#ifndef TILTEDLAYERSVISITOR_HH
#define	TILTEDLAYERSVISITOR_HH

#include <vector>

#include "ConstGeometryVisitor.hh"

class RootWTable;
class Layer;

    //***************************************//
    //*                Visitor              *//
    //* Automatic-placement tilted layers : *//
    //*            Additional info          *//
    //*                                     *//
    //***************************************//

class TiltedLayersVisitor : public ConstGeometryVisitor {
public:
  // tilted info
  std::vector<RootWTable*> tiltedLayerNames;
  std::vector<RootWTable*> flatPartNames;
  std::vector<RootWTable*> tiltedPartNames;
  std::vector<RootWTable*> flatPartTables;
  std::vector<RootWTable*> tiltedPartTables;

  // counter
  int numTiltedLayers = 0;

  void visit(const Layer& l) override;     
};

#endif
