#include "flow-point-to-point-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FlowPointToPointNetDevice");

TypeId FlowPointToPointNetDevice::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::FlowPointToPointNetDevice")
  .SetParent<NetDevice>()
    .AddConstructor<FlowPointToPointNetDevice>()
      .AddAttribute("Address",
          "The MAC address of this device.",
          Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
          MakeMac48AddressAccessor (&FlowPointToPointNetDevice::m_address),
          MakeMac48AddressChecker ())
      .AddAttribute ("DataRate",
          "The default data rate for point to point links",
          DataRateValue (DataRate ("32768b/s")),
          MakeDataRateAccessor (&FlowPointToPointNetDevice::m_bps),
          MakeDataRateChecker ())
       .AddAttribute ("InterframeGap",
          "The time to wait between packet (frame) transmissions",
          TimeValue (Seconds (0.0)),
          MakeTimeAccessor (&FlowPointToPointNetDevice::m_tInterframeGap),
          MakeTimeChecker ());
  return tid;
}

FlowPointToPointNetDevice::FlowPointToPointNetDevice() :
    m_channel(0),
    m_linkUp(false){
    NS_LOG_FUNCTION(this);
}

FlowPointToPointNetDevice::~FlowPointToPointNetDevice() {
    NS_LOG_FUNCTION(this);
}

void FlowPointToPointNetDevice::SetDataRate(DataRate bps)
{
    NS_LOG_FUNCTION (this);
    m_bps = bps;
}

DataRate FlowPointToPointNetDevice::GetDataRate() {
    return m_bps;
}

bool FlowPointToPointNetDevice::Attach (Ptr<FlowPointToPointChannel> ch) {
    NS_LOG_FUNCTION (this << &ch);

    m_channel = ch;

    m_channel->Attach (this);

    //
    // This device is up whenever it is attached to a channel.  A better plan
    // would be to have the link come up when both devices are attached, but this
    // is not done for now.
    //
    NotifyLinkUp ();
    return true;
}

void FlowPointToPointNetDevice::SetFlowQueue (Ptr<FlowQueue> q) {
    NS_LOG_FUNCTION (this << q);
    m_queue = q;
}

Ptr<FlowQueue> FlowPointToPointNetDevice::GetFlowQueue (void) const {
    NS_LOG_FUNCTION (this);
    return m_queue;
}

void FlowPointToPointNetDevice::NotifyLinkUp(void) {
    NS_LOG_FUNCTION (this);
    m_linkUp = true;
    m_linkChangeCallbacks ();
}

void Receive(Ptr<Flow> flow) {
    /*
     * To be implemented
     */
}

void FlowPointToPointNetDevice::SetIfIndex (const uint32_t index) {
    NS_LOG_FUNCTION (this);
    m_ifIndex = index;
}

uint32_t FlowPointToPointNetDevice::GetIfIndex (void) const {
    return m_ifIndex;
}

Ptr<Channel>
FlowPointToPointNetDevice::GetChannel (void) const {
    return m_channel;
}

void FlowPointToPointNetDevice::SetAddress (Address address) {
    NS_LOG_FUNCTION (this << address);
    m_address = Mac48Address::ConvertFrom (address);
}

Address FlowPointToPointNetDevice::GetAddress (void) const {
    return m_address;
}

bool FlowPointToPointNetDevice::SetMtu (uint16_t mtu) {
    NS_LOG_FUNCTION (this << mtu);
    m_mtu = mtu;
    return true;
}

uint16_t FlowPointToPointNetDevice::GetMtu (void) const {
    NS_LOG_FUNCTION (this);
    return m_mtu;
}

bool FlowPointToPointNetDevice::IsLinkUp (void) const {
    NS_LOG_FUNCTION (this);
    return m_linkUp;
}

void FlowPointToPointNetDevice::AddLinkChangeCallback (Callback<void> callback) {
    NS_LOG_FUNCTION (this);
    m_linkChangeCallbacks.ConnectWithoutContext (callback);
}

bool FlowPointToPointNetDevice::IsBroadcast (void) const {
    NS_LOG_FUNCTION (this);
    return true;
}

Address FlowPointToPointNetDevice::GetBroadcast (void) const {
    NS_LOG_FUNCTION (this);
    return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool FlowPointToPointNetDevice::IsMulticast (void) const {
    NS_LOG_FUNCTION (this);
    return true;
}

Address FlowPointToPointNetDevice::GetMulticast (Ipv4Address multicastGroup) const {
    NS_LOG_FUNCTION (this);
    return Mac48Address ("01:00:5e:00:00:00");
}

Address FlowPointToPointNetDevice::GetMulticast (Ipv6Address addr) const {
    NS_LOG_FUNCTION (this << addr);
    return Mac48Address ("33:33:00:00:00:00");
}

bool FlowPointToPointNetDevice::IsPointToPoint (void) const {
    NS_LOG_FUNCTION (this);
    return true;
}

bool FlowPointToPointNetDevice::IsBridge (void) const {
    NS_LOG_FUNCTION (this);
    return false;
}

bool FlowPointToPointNetDevice::Send (
    Ptr<Packet> packet,
    const Address &dest,
    uint16_t protocolNumber) {
  return false;
}

bool FlowPointToPointNetDevice::SendFrom (Ptr<Packet> packet,
    const Address &source,
    const Address &dest,
    uint16_t protocolNumber) {
    NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
  return false;
}

Ptr<Node> FlowPointToPointNetDevice::GetNode (void) const {
    return m_node;
}

void FlowPointToPointNetDevice::SetNode (Ptr<Node> node) {
    NS_LOG_FUNCTION (this);
    m_node = node;
}

bool FlowPointToPointNetDevice::NeedsArp (void) const {
    NS_LOG_FUNCTION (this);
    return false;
}

void FlowPointToPointNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb) {
    m_rxCallback = cb;
}

void FlowPointToPointNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb) {
    m_promiscCallback = cb;
}

bool FlowPointToPointNetDevice::SupportsSendFrom (void) const {
    NS_LOG_FUNCTION (this);
    return false;
}

} // namespace ns3