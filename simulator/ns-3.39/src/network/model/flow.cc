#include "flow.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Flow");

NS_OBJECT_ENSURE_REGISTERED(Flow);

TypeId Flow::GetTypeId() {
  static TypeId tid = TypeId("ns3::Flow")
      .SetParent<Object>()
      .AddConstructor<Flow>();
  return tid;
}

Flow::Flow()
    : m_fiveTuple(),
      m_priority() {}

Flow::~Flow() {}

Flow::Flow(Flow::FiveTuple fiveTuple, uint8_t priority)
    : m_fiveTuple(fiveTuple),
      m_priority(priority) {}

Flow::Flow(const ns3::Flow& o)
    : m_fiveTuple(o.m_fiveTuple),
      m_priority(o.m_priority) {}

Flow& Flow::operator=(const Flow& o) {
    if (this == &o) {
        return *this;
    }
    m_fiveTuple = o.m_fiveTuple;
    m_priority = o.m_priority;
    return *this;
}

Flow::FiveTuple Flow::GetFiveTuple() const {
    return m_fiveTuple;
}

Flow::FiveTuple Flow::SetFiveTuple(FiveTuple fiveTuple) {
    m_fiveTuple = fiveTuple;
}

uint8_t Flow::GetPriority() const {
    return m_priority;
}

bool Flow::FiveTuple::operator<(const ns3::Flow::FiveTuple& other) const {
    return std::tie(sourceAddress, destinationAddress, sourcePort, destinationPort, protocol) <
           std::tie(other.sourceAddress, other.destinationAddress, other.sourcePort, other.destinationPort, other.protocol);
}

bool Flow::FiveTuple::operator==(const ns3::Flow::FiveTuple& other) const {
    return std::tie(sourceAddress, destinationAddress, sourcePort, destinationPort, protocol) ==
           std::tie(other.sourceAddress, other.destinationAddress, other.sourcePort, other.destinationPort, other.protocol);
}

bool operator<(const Flow& f1, const Flow& f2) {
    return f1.m_fiveTuple < f2.m_fiveTuple;
}

bool operator==(const Flow& f1, const Flow& f2) {
    return f1.m_fiveTuple == f2.m_fiveTuple;
}

} // namespace ns3