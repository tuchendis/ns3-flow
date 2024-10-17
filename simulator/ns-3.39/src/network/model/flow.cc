#include "flow.h"
#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

#include <stdint.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Flow");

Flow::Flow()
    : m_fiveTuple(),
      m_rate(),
      m_priority() {}

Flow::Flow(Flow::FiveTuple fiveTuple, uint32_t rate, uint8_t priority)
    : m_fiveTuple(fiveTuple),
      m_rate(rate),
      m_priority(priority) {}

Flow::Flow(const ns3::Flow& o)
    : m_fiveTuple(o.m_fiveTuple),
      m_rate(o.m_rate),
      m_priority(o.m_priority) {}

Flow& Flow::operator=(const Flow& o) {
    if (this == &o) {
        return *this;
    }
    m_fiveTuple = o.m_fiveTuple;
    m_rate = o.m_rate;
    m_priority = o.m_priority;
    return *this;
}

} // namespace ns3