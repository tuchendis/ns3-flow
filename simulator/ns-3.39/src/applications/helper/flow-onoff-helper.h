#ifndef FLOW_ON_OFF_HELPER_H
#define FLOW_ON_OFF_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/flow-onoff-application.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class DataRate;

class FlowOnOffHelper {
  public:
    FlowOnOffHelper(std::string protocol, Address srcAddr, Address dstAddr);

    void SetAttribute(std::string name, const AttributeValue& value);

    void SetConstantRate(DataRate dataRate);

    ApplicationContainer Install(NodeContainer c) const;

    ApplicationContainer Install(Ptr<Node> node) const;

  private:
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory;
};

} // namespace ns3

#endif /* FLOW_ON_OFF_HELPER_H */