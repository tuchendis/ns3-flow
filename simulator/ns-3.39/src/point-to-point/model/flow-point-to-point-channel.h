#ifndef FLOW_POINT_TO_POINT_CHANNEL_H
#define FLOW_POINT_TO_POINT_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/logical-flow-channel.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/ptr.h"
#include "ns3/qbb-net-device.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

#include <vector>

namespace ns3
{

class PointToPointNetDevice;
class Flow;

class FlowPointToPointChannel : public Channel {
  public:
    static TypeId GetTypeId(void);

    FlowPointToPointChannel();

    void Attach(Ptr<QbbNetDevice> device);

    std::size_t GetNDevices() const;

    Ptr<NetDevice> GetDevice(std::size_t i) const;

  private:
    static const std::size_t N_DEVICES = 2;
    static const std::size_t N_CHANNELS = 2;

    std::size_t m_nDevices;
    std::vector<Ptr<Channel>> m_channels;
};

} // namespace ns3

#endif /* FLOW_POINT_TO_POINT_CHANNEL_H */