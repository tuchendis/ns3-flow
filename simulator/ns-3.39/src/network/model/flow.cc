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

bool operator<(const Flow::FiveTuple& t1, const Flow::FiveTuple& t2) {
    if (t1.sourceAddress < t2.sourceAddress) {
        return true;
    }
    if (t1.sourceAddress != t2.sourceAddress) {
        return false;
    }

    if (t1.destinationAddress < t2.destinationAddress) {
        return true;
    }
    if (t1.destinationAddress != t2.destinationAddress) {
        return false;
    }

    if (t1.protocol < t2.protocol) {
        return true;
    }
    if (t1.protocol != t2.protocol) {
        return false;
    }

    if (t1.sourcePort < t2.sourcePort) {
        return true;
    }
    if (t1.sourcePort != t2.sourcePort) {
        return false;
    }

    if (t1.destinationPort < t2.destinationPort) {
        return true;
    }
    if (t1.destinationPort != t2.destinationPort) {
        return false;
    }

    return false;
}

bool operator==(const Flow::FiveTuple& t1, const Flow::FiveTuple& t2) {
  return (t1.sourceAddress == t2.sourceAddress &&
      t1.destinationAddress == t2.destinationAddress &&
      t1.protocol == t2.protocol &&
      t1.sourcePort == t2.sourcePort &&
      t1.destinationPort == t2.destinationPort);
}

} // namespace ns3