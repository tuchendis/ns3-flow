#ifndef FLOW_POINT_TO_POINT_CHANNEL_H
#define FLOW_POINT_TO_POINT_CHANNEL_H

#include "ns3/channel.h"
#include "flow-point-to-point-net-device.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

#include <vector>

namespace ns3
{

class FlowPointToPointNetDevice;
class Flow;

class FlowPointToPointChannel : public Channel {
  public:
    static TypeId GetTypeId(void);

    FlowPointToPointChannel();

    void Attach(Ptr<FlowPointToPointNetDevice> device);

    virtual std::size_t GetNDevices() const;

    virtual Ptr<NetDevice> GetDevice(std::size_t i) const;

    Ptr<FlowPointToPointNetDevice> GetFlowPointToPointDevice (std::size_t i) const;

  private:
    static const std::size_t N_DEVICES = 2;

    std::size_t m_nDevices;
    std::vector<Channel> m_channels;

    enum WireState {
      /** Initializing state */
      INITIALIZING,
      /** Idle state (no transmission from NetDevice) */
      IDLE,
      /** Transmitting state (data being transmitted from NetDevice. */
      TRANSMITTING,
      /** Propagating state (data is being propagated in the channel. */
      PROPAGATING
    };

    class Link {
      public:
        /** \brief Create the link, it will be in INITIALIZING state
         *
         */
        Link() : m_state (INITIALIZING), m_src (0), m_dst (0) {}

        WireState m_state; //!< State of the link
        Ptr<FlowPointToPointNetDevice> m_src;   //!< First NetDevice
        Ptr<FlowPointToPointNetDevice> m_dst;   //!< Second NetDevice
    };

    Link m_link[N_DEVICES]; //!< Link model
};

} // namespace ns3

#endif /* FLOW_POINT_TO_POINT_CHANNEL_H */