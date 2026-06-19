#ifndef DD4HEP_COMPACT_WRITER_HH
#define DD4HEP_COMPACT_WRITER_HH

#include <fstream>
#include <string>

namespace insur {

struct CMSSWBundle;
struct XmlTags;

class DD4hepCompactWriter {
public:
  void tracker(CMSSWBundle& d, std::ofstream& out,
                bool isPixelTracker, XmlTags& tags);

  void setSimpleHeader(const std::string& hdr) { simpleHeader_ = hdr; }

private:
  std::string simpleHeader_;
};

}  // namespace insur

#endif
