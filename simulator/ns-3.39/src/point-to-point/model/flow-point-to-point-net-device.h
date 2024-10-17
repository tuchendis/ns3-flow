#ifndef FLOW_NET_DEVICE_H
#define FLOW_NET_DEVICE_H

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/data-rate.h"
#include "ns3/flow-queue.h"
#include "ns3/flow-point-to-point-channel.h"
#include "ns3/ipv4-address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <stdint.h>

namespace ns3 {

class RdmaMixEgressQueue : public Object {
  public:
    static const uint32_t qCnt = 8;
    static uint32_t ack_q_idx;
    static uint32_t tcpip_q_idx;

};

class FlowPointToPointChannel;

class FlowPointToPointNetDevice : public NetDevice {
  public:
    static TypeId GetTypeId(void);

    FlowPointToPointNetDevice();
    virtual ~FlowPointToPointNetDevice();

    void SetDataRate(DataRate bps);

    DataRate GetDataRate();

    bool Attach(Ptr<FlowPointToPointChannel> ch);

    void SetFlowQueue(Ptr<FlowQueue> queue);

    Ptr<FlowQueue> GetFlowQueue(void) const;

    void Receive(Ptr<Flow> flow);

    virtual void SetIfIndex (const uint32_t index);
    virtual uint32_t GetIfIndex (void) const;

    virtual Ptr<Channel> GetChannel (void) const;

    virtual void SetAddress (Address address);
    virtual Address GetAddress (void) const;

    virtual bool SetMtu (const uint16_t mtu);
    virtual uint16_t GetMtu (void) const;

    virtual bool IsLinkUp (void) const;

    virtual void AddLinkChangeCallback (Callback<void> callback);

    virtual bool IsBroadcast (void) const;
    virtual Address GetBroadcast (void) const;

    virtual bool IsMulticast (void) const;
    virtual Address GetMulticast (Ipv4Address multicastGroup) const;
    virtual Address GetMulticast(Ipv6Address addr) const;

    virtual bool IsPointToPoint (void) const;
    virtual bool IsBridge (void) const;

    virtual bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
    virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);

    virtual Ptr<Node> GetNode (void) const;
    virtual void SetNode (Ptr<Node> node);

    virtual bool NeedsArp (void) const;

    virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

    virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
    virtual bool SupportsSendFrom (void) const;

  private:
    FlowPointToPointNetDevice& operator = (const FlowPointToPointNetDevice& o);

    FlowPointToPointNetDevice(const FlowPointToPointNetDevice &o);

    virtual void DoDispose(void);

    void NotifyLinkUp (void);

    DataRate m_bps;
    Time m_tInterframeGap;
    Ptr<FlowPointToPointChannel> m_channel;
    Ptr<FlowQueue> m_queue;

    Ptr<Node> m_node;
    Mac48Address m_address;
    NetDevice::ReceiveCallback m_rxCallback;
    NetDevice::PromiscReceiveCallback m_promiscCallback;

    uint32_t m_ifIndex;
    bool m_linkUp;
    TracedCallback<> m_linkChangeCallbacks;

    static const uint16_t DEFAULT_MTU = 1500;

    uint32_t m_mtu;
};

} // namespace ns

#endif /* FLOW_NET_DEVICE_H */