#ifndef LOGICAL_FLOW_CHANNEL_H
#define LOGICAL_FLOW_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/flow-point-to-point-channel.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class FlowPointToPointChannel;

class LogicalFlowChannel : public Object {
  public:
    static TypeId GetTypeId(void);

    LogicalFlowChannel();

    void Attach (Ptr<FlowPointToPointChannel> flowChannel);
};

} // namespace ns3

#endif