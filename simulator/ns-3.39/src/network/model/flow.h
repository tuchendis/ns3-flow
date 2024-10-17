#ifndef FLOW_H
#define FLOW_H

#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

#include <stdint.h>

namespace ns3 {

class Flow : SimpleRefCount<Flow> {
  public:
    struct FiveTuple
    {
        Ipv4Address sourceAddress;      //!< Source address
        Ipv4Address destinationAddress; //!< Destination address
        uint8_t protocol;               //!< Protocol
        uint16_t sourcePort;            //!< Source port
        uint16_t destinationPort;       //!< Destination port
    };

    Flow();
    Flow(FiveTuple fiveTuple, uint32_t rate, uint8_t priority);
    ~Flow();

    Flow(const Flow& o);
    Flow& operator=(const Flow& o);

    inline const FiveTuple& GetTuple() const;
    inline uint32_t GetRate() const;
    inline uint8_t GetPriority() const;
  private:
    FiveTuple m_fiveTuple;
    uint32_t m_rate;
    uint8_t m_priority;
};

} // namespace ns3

/****************************************************
 *  Implementation of inline methods for performance
 ****************************************************/

namespace ns3 {

const Flow::FiveTuple& Flow::GetTuple() const {
    return m_fiveTuple;
}

uint32_t Flow::GetRate() const {
    return m_rate;
}

uint8_t Flow::GetPriority() const {
    return m_priority;
}

} // namespace ns3

#endif /* FLOW_H */