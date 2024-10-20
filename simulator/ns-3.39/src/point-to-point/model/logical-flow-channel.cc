#include "logical-flow-channel.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/flow.h"

namespace ns3 {

TypeId LogicalFlowChannel::GetTypeId() {
  static TypeId tid = TypeId("ns3::LogicalFlowChannel")
      .SetParent<Object>()
      .AddConstructor<LogicalFlowChannel>();
  return tid;
}

LogicalFlowChannel::LogicalFlowChannel() :
    m_nDevices(0),
    m_links(),
    m_delay(Seconds (0.)),
    m_bps(0),
    m_flows() {}

std::size_t LogicalFlowChannel::GetNDevices() const {
  return m_nDevices;
}

Ptr<NetDevice> LogicalFlowChannel::GetDevice(std::size_t i) const {
  NS_ASSERT (i < 2);
  return m_links[i].m_src;
}

void LogicalFlowChannel::Attach(Ptr<ns3::FlowPointToPointChannel> flowChannel) {
  m_flowChannel = flowChannel;
}

void LogicalFlowChannel::Attach(Ptr<ns3::PointToPointNetDevice> netDevice) {
  m_links[m_nDevices++].m_src = netDevice;
  if (m_nDevices == N_DEVICES) {
    m_links[0].m_dst = m_links[1].m_dst;
    m_links[1].m_dst = m_links[0].m_src;
  }
}

bool LogicalFlowChannel::Transmit(Ptr<ns3::Flow> flow, DataRate rate) {
  auto result = m_flows.insert_or_assign(flow->GetFiveTuple(), rate);
  return result.second;
}

} // namespace ns3