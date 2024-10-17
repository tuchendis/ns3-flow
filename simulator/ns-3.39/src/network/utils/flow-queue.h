#ifndef FLOW_QUEUE_H
#define FLOW_QUEUE_H

#include "ns3/callback.h"
#include "ns3/flow.h"
#include "ns3/ipv4-address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/queue-size.h"

#include <vector>

namespace ns3 {

class FlowQueue : public Object {
  public:
    static TypeId GetTypeId();

    FlowQueue();
    ~FlowQueue() override;

    bool IsEmpty() const;

    void SetMaxSize(QueueSize size);

    QueueSize GetMaxSizee() const;

    void DoFlowEnqueue(Ptr<Flow> flow);

    void DoFlowDequeue(Ptr<Flow> flow);

  private:

    QueueSize m_maxSize;
    std::vector<Ptr<Flow>> m_flows;
};

} // namespace ns3

#endif
