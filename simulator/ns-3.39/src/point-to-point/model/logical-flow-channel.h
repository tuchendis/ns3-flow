#ifndef LOGICAL_FLOW_CHANNEL_H
#define LOGICAL_FLOW_CHANNEL_H

#include "ns3/flow.h"
#include "ns3/channel.h"
#include "ns3/flow-point-to-point-channel.h"
#include "ns3/ptr.h"
#include "ns3/qbb-net-device.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"

#include <map>

namespace ns3 {

class Flow;
class QbbNetDevice;
class FlowPointToPointChannel;

class LogicalFlowChannel : public Channel {
  public:
    static TypeId GetTypeId(void);

    LogicalFlowChannel();

    std::size_t GetNDevices() const;

    Ptr<NetDevice> GetDevice(std::size_t i) const;

    void Attach(Ptr<FlowPointToPointChannel> flowChannel);

    void Attach(Ptr<QbbNetDevice> netDevice);

    void CalculateBandWidths();

    void Transmit(Ptr<Flow> flow, DataRate rate, Ptr<QbbNetDevice> src);

  private:
    static const std::size_t N_DEVICES = 2;
    std::size_t m_nDevices;

    Time m_delay;
    DataRate m_bps; // channel rate
    Ptr<FlowPointToPointChannel> m_flowChannel;
    Ptr<QbbNetDevice> m_netDevice;

    std::map<Ptr<Flow>, DataRate> m_flows; // Output flow rate
    DataRate m_totalInputRate;

    class Link {
      public:
        /** \brief Create the flow link
         *
         */
        Link() : m_src (), m_dst () {}

        Ptr<QbbNetDevice> m_src;   //!< First NetDevice
        Ptr<QbbNetDevice> m_dst;   //!< Second NetDevice
    };

    Link m_links[N_DEVICES];
};
} // namespace ns3

#endif