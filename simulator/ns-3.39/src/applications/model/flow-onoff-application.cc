#include "ns3/flow-onoff-application.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FlowOnOffApplication");

NS_OBJECT_ENSURE_REGISTERED(FlowOnOffApplication);

TypeId FlowOnOffApplication::GetTypeId() {
    static TypeId tid =
        TypeId("ns3::FlowOnOffApplication")
            .SetParent<OnOffApplication>()
            .SetGroupName("Applications")
            .AddConstructor<FlowOnOffApplication>();
    return tid;
}

FlowOnOffApplication::FlowOnOffApplication() {
    NS_LOG_FUNCTION(this);
}

FlowOnOffApplication::~FlowOnOffApplication() {
    NS_LOG_FUNCTION(this);
}

void FlowOnOffApplication::StartApplication() {
    NS_LOG_FUNCTION(this);
    if (!m_socket) {
        m_socket = Socket::CreateSocket(GetNode(), m_tid);

        int ret = -1;
        if (!m_local.IsInvalid()) {
            NS_ABORT_MSG_IF((Inet6SocketAddress::IsMatchingType(m_peer) &&
                             InetSocketAddress::IsMatchingType(m_local)) ||
                                (InetSocketAddress::IsMatchingType(m_peer) &&
                                 Inet6SocketAddress::IsMatchingType(m_local)),
                            "Incompatible peer and local address IP version");
            ret = m_socket->Bind(m_local);
        } else {
            if (Inet6SocketAddress::IsMatchingType(m_peer)) {
                ret = m_socket->Bind6();
            }
            else if (InetSocketAddress::IsMatchingType(m_peer) ||
                     PacketSocketAddress::IsMatchingType(m_peer)) {
                ret = m_socket->Bind();
            }
        }

        if (ret == -1) {
            NS_FATAL_ERROR("Failed to bind socket");
        }

        m_socket->SetConnectCallback(MakeCallback(&FlowOnOffApplication::ConnectionSucceeded, this),
                                     MakeCallback(&FlowOnOffApplication::ConnectionFailed, this));

        m_socket->Connect(m_peer);
        m_socket->SetAllowBroadcast(true);
        m_socket->ShutdownRecv();
    }

    // Ensure no pending event
    CancelEvents();

    if (m_connected) {
        ScheduleStartEvent();
    }
}

void FlowOnOffApplication::StopApplication() {
    NS_LOG_FUNCTION(this);

    CancelEvents();
    if (m_socket) {
        m_socket->Close();
    }
    else {
        NS_LOG_WARN("FlowOnOffApplication found null socket to close in StopApplication");
    }
}

void FlowOnOffApplication::StartSending() {
    NS_LOG_FUNCTION(this);
    m_lastStartTime = Simulator::Now();

//    if (m_totalTime < m_maxTime)
//    else
//    { // All done, cancel any pending events
//        StopApplication();
//    }

//    Time nextTime(
//        Seconds(bits / static_cast<double>(m_cbrRate.GetBitRate()))); // Time till next flow
    SendFlow();
//    ScheduleStopEvent();
}

void FlowOnOffApplication::SendFlow() {
    Ptr<Flow> flow = CreateObject<Flow>();
    m_socket->SendFlow(flow, m_cbrRate);
}

void FlowOnOffApplication::StopSending() {
    NS_LOG_FUNCTION(this);
    m_lastStartTime = Simulator::Now();
    ScheduleStartEvent();
}

void FlowOnOffApplication::CancelEvents() {
    NS_LOG_FUNCTION(this);

    Simulator::Cancel(m_sendEvent);
    Simulator::Cancel(m_startStopEvent);
}

void FlowOnOffApplication::ScheduleStartEvent() { // Schedules the event to start sending data (switch to the "On" state)
    NS_LOG_FUNCTION(this);

    Time offInterval = Seconds(m_offTime->GetValue());
    NS_LOG_LOGIC("start at " << offInterval.As(Time::S));
    m_startStopEvent = Simulator::Schedule(offInterval, &FlowOnOffApplication::StartSending, this);
}

void FlowOnOffApplication::ScheduleStopEvent() { // Schedules the event to stop sending data (switch to "Off" state)
    NS_LOG_FUNCTION(this);

    Time onInterval = Seconds(m_onTime->GetValue());
    NS_LOG_LOGIC("stop at " << onInterval.As(Time::S));
    m_startStopEvent = Simulator::Schedule(onInterval, &FlowOnOffApplication::StopSending, this);
}

void FlowOnOffApplication::ConnectionSucceeded(Ptr<ns3::Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    ScheduleStartEvent();
    m_connected = true;
}

void FlowOnOffApplication::ConnectionFailed(Ptr<ns3::Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_FATAL_ERROR("Can't connect.");
}

} // namespace ns3
