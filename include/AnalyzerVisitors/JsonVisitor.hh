#ifndef JSONVISITOR_HH
#define JSONVISITOR_HH

#include <boost/json.hpp>

class Tracker;
class Barrel;
class Endcap;
class Disk;
class Layer;
class RodPair;
class Ring;
class DetectorModule;

class JsonVisitor
{
public:
    boost::json::object build(const Tracker *t, const Tracker *p);
private:
    boost::json::object visit_tracker(const Tracker &t);
    boost::json::object visit_barrel(const Barrel &b);
    boost::json::object visit_endcap(const Endcap &e);
    boost::json::object visit_disk(const Disk &d);
    boost::json::object visit_layer(const Layer &l);
    boost::json::object visit_rodpairs(const RodPair& r);
    boost::json::object visit_ring(const Ring &r);
    boost::json::object visit_module(const DetectorModule &m);
};

#endif
