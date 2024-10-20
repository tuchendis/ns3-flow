#ifndef LOGICAL_FLOW_CHANNEL_H
#define LOGICAL_FLOW_CHANNEL_H

#include "ns3/flow.h"
#include "ns3/channel.h"
#include "ns3/flow-point-to-point-channel.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"

#include <map>

namespace ns3 {

class Flow;
class FlowPointToPointChannel;

class LogicalFlowChannel : public Channel {
  public:
    static TypeId GetTypeId(void);

    LogicalFlowChannel();

    std::size_t GetNDevices() const;

    Ptr<NetDevice> GetDevice(std::size_t i) const;

    void Attach (Ptr<FlowPointToPointChannel> flowChannel);

    void Attach (Ptr<PointToPointNetDevice> netDevice);

    bool Transmit (Ptr<Flow> flow, DataRate rate);

  private:
    static const std::size_t N_DEVICES = 2;
    std::size_t m_nDevices;

    Time m_delay;
    DataRate m_bps;
    Ptr<FlowPointToPointChannel> m_flowChannel;
    Ptr<PointToPointNetDevice> m_netDevice;

    std::map<Flow::FiveTuple, DataRate> m_flows; // Output flow rate

    class Link {
      public:
        /** \brief Create the flow link
         *
         */
        Link() : m_src (0), m_dst (0) {}

        Ptr<PointToPointNetDevice> m_src;   //!< First NetDevice
        Ptr<PointToPointNetDevice> m_dst;   //!< Second NetDevice
    };

    Link m_links[N_DEVICES];
};
} // namespace ns3

#endif