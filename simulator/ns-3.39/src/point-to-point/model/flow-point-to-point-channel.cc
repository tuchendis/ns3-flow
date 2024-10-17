#include "flow-point-to-point-channel.h"
#include "flow-point-to-point-net-device.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FlowPointToPointChannel");

TypeId FlowPointToPointChannel::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::FlowPointToPointChannel")
      .SetParent<Channel>()
      .AddConstructor<FlowPointToPointChannel> ();
  return tid;
}

FlowPointToPointChannel::FlowPointToPointChannel()
  : m_channels() {
    NS_LOG_FUNCTION_NOARGS();
}

Ptr<FlowPointToPointNetDevice>
FlowPointToPointChannel::GetFlowPointToPointDevice (std::size_t i) const {
    NS_LOG_FUNCTION_NOARGS ();
    NS_ASSERT (i < 2);
    return m_link[i].m_src;
}

Ptr<NetDevice>
FlowPointToPointChannel::GetDevice(std::size_t i) const {
  NS_LOG_FUNCTION_NOARGS ();
  return m_link[i].m_src;
}

std::size_t FlowPointToPointChannel::GetNDevices(void) const {
  NS_LOG_FUNCTION_NOARGS();
  return m_nDevices;
}

} // namespace ns3