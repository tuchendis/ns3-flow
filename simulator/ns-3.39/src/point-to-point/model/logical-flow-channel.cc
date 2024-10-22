#include "ns3/assert.h"
#include "ns3/flow.h"
#include "ns3/log.h"
#include "ns3/logical-flow-channel.h"
#include "ns3/simulator.h"
#include "ns3/switch-node.h"

namespace ns3 {

class Flow;
class DataRate;
class QbbNetDevice;

NS_OBJECT_ENSURE_REGISTERED(LogicalFlowChannel);

TypeId LogicalFlowChannel::GetTypeId() {
  static TypeId tid = TypeId("ns3::LogicalFlowChannel")
      .SetParent<Object>()
      .AddConstructor<LogicalFlowChannel>()
      .AddAttribute("DataRate", "Propagation delay through the channel",
          DataRateValue( 0),
          MakeDataRateAccessor(&LogicalFlowChannel::m_bps),
          MakeDataRateChecker());
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

void LogicalFlowChannel::Attach(Ptr<FlowPointToPointChannel> flowChannel) {
  m_flowChannel = flowChannel;
}

void LogicalFlowChannel::Attach(Ptr<QbbNetDevice> netDevice) {
  m_links[m_nDevices++].m_src = netDevice;
  if (m_nDevices == N_DEVICES) {
    m_links[0].m_dst = m_links[1].m_src;
    m_links[1].m_dst = m_links[0].m_src;
  }
}

void LogicalFlowChannel::CalculateBandWidths() {

}

void LogicalFlowChannel::Transmit(Ptr<Flow> flow, DataRate rate, Ptr<QbbNetDevice> src) {
    uint32_t wire = src == m_links[0].m_src ? 0 : 1;
    m_totalInputRate -= m_flows[flow];
    m_totalInputRate += rate;

    if (rate == 0) {
        m_flows.erase(flow);
    } else {
        std::cout << rate / src->m_totalEngressRate << std::endl;
        m_flows.insert_or_assign(flow, rate / src->m_totalEngressRate * m_bps);
    }

    Simulator::ScheduleWithContext(m_links[wire].m_dst->GetNode ()->GetId (),
            m_delay,
            &QbbNetDevice::ReceiveFlow,
            m_links[wire].m_dst,
            m_flows);
}

} // namespace ns3