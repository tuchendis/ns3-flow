#include "flow-onoff-helper.h"

#include "ns3/data-rate.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/flow-onoff-application.h"
#include "ns3/packet-socket-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

namespace ns3 {

FlowOnOffHelper::FlowOnOffHelper(std::string protocol, ns3::Address srcAddr, ns3::Address dstAddr) {
    m_factory.SetTypeId("ns3::FlowOnOffApplication");
    m_factory.Set("Protocol", StringValue(protocol));
    m_factory.Set("Local", AddressValue(srcAddr));
    m_factory.Set("Remote",  AddressValue(dstAddr));
}

void FlowOnOffHelper::SetAttribute(std::string name, const ns3::AttributeValue& value) {
    m_factory.Set(name, value);
}

ApplicationContainer FlowOnOffHelper::Install(Ptr<ns3::Node> node) const {
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer FlowOnOffHelper::Install(NodeContainer c) const {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application> FlowOnOffHelper::InstallPriv(Ptr<ns3::Node> node) const {
    Ptr<Application> app = m_factory.Create<Application>();
    node->AddApplication(app);

    return app;
}

void FlowOnOffHelper::SetConstantRate(DataRate dataRate) {
    m_factory.Set("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1000]"));
    m_factory.Set("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    m_factory.Set("DataRate", DataRateValue(dataRate));
}

} // namespace ns3