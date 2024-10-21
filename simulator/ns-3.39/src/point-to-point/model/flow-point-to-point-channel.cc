#include "ns3/flow-point-to-point-channel.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FlowPointToPointChannel");

NS_OBJECT_ENSURE_REGISTERED(FlowPointToPointChannel);

TypeId FlowPointToPointChannel::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::FlowPointToPointChannel")
      .SetParent<Channel>()
      .AddConstructor<FlowPointToPointChannel> ();
  return tid;
}

std::size_t FlowPointToPointChannel::GetNDevices() const {
  return N_DEVICES;
}

Ptr<NetDevice> FlowPointToPointChannel::GetDevice(std::size_t i) const {
  return DynamicCast<PointToPointChannel>(m_channels[0])->GetDevice(i);
}

void FlowPointToPointChannel::Attach(Ptr<ns3::QbbNetDevice> device) {
  DynamicCast<PointToPointChannel>(m_channels[0])->Attach(device);
  DynamicCast<LogicalFlowChannel>(m_channels[1])->Attach(device);
}

FlowPointToPointChannel::FlowPointToPointChannel()
  : m_nDevices(N_DEVICES),
    m_channels(N_CHANNELS) {
  NS_LOG_FUNCTION_NOARGS();
  m_channels.push_back(CreateObject<PointToPointChannel>());
  m_channels.push_back(CreateObject<LogicalFlowChannel>());
}

} // namespace ns3