#ifndef TK2DD4HEP_HH
#define TK2DD4HEP_HH

#include <string>

#include "tk2CMSSW_datatypes.hh"
#include "Extractor.hh"
#include "DD4hepCompactWriter.hh"

class mainConfigHandler;

namespace insur {

class MaterialTable;
class MaterialBudget;

/**
 * @class tk2DD4hep
 * @brief Orchestrates DD4hep compact XML generation from a tracker material budget.
 *
 * Called once per sub-detector (OT and IT) from Squid::translateFullSystemToXML,
 * writing tracker_dd4hep.xml and pixel_dd4hep.xml into the output directory.
 */
class tk2DD4hep {
public:
  explicit tk2DD4hep(mainConfigHandler& mch) : mainConfiguration(mch) {}
  virtual ~tk2DD4hep() {}

  void translate(MaterialTable& mt, MaterialBudget& mb,
                  XmlTags& trackerXmlTags,
                  const std::string& xmlOutputPath,
                  const std::string& xmlOutputName = "");

private:
  CMSSWBundle         trackerData;
  CMSSWBundle         otstData;
  Extractor           ex;
  DD4hepCompactWriter wr;
  mainConfigHandler& mainConfiguration;
};

}  // namespace insur

#endif
