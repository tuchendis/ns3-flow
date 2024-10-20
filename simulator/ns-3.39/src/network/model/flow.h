#ifndef FLOW_H
#define FLOW_H

#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"

#include <stdint.h>

namespace ns3 {

class Flow : public Object {
  public:
    static TypeId GetTypeId();

    struct FiveTuple {
        Ipv4Address sourceAddress;      //!< Source address
        Ipv4Address destinationAddress; //!< Destination address
        uint16_t sourcePort;            //!< Source port
        uint16_t destinationPort;       //!< Destination port
        uint8_t protocol;               //!< Protocol

      FiveTuple(Ipv4Address src, Ipv4Address dst, uint16_t srcPort, uint16_t dstPort, uint8_t proto)
            : sourceAddress(src),
              destinationAddress(dst),
              sourcePort(srcPort),
              destinationPort(dstPort),
              protocol(proto) {}
    };

    Flow(void);
    Flow(FiveTuple fiveTuple, uint8_t priority);
    ~Flow();

    Flow(const Flow& o);
    Flow& operator=(const Flow& o);

    FiveTuple GetFiveTuple() const;
    FiveTuple SetFiveTuple(FiveTuple fiveTuple);
    uint8_t GetPriority() const;
  private:
    FiveTuple m_fiveTuple;
    uint8_t m_priority;
};

bool operator<(const Flow::FiveTuple& t1, const Flow::FiveTuple& t2);

bool operator==(const Flow::FiveTuple& t1, const Flow::FiveTuple& t2);

} // namespace ns3

#endif /* FLOW_H */