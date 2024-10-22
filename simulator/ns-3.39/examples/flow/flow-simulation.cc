#include <iostream>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include "ns3/core-module.h"
#include "ns3/qbb-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/packet.h"
#include "ns3/error-model.h"
#include <ns3/switch-node.h>
#include <ns3/sim-setting.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("GENERIC_SIMULATION");

uint32_t packet_payload_size = 1000;

NodeContainer nodes;

/************************************************
 * Runtime variables
 ***********************************************/
std::ifstream topof, flowf, tracef;

NetDeviceContainer switchToSwitchInterfaces;
std::map< uint32_t, std::map< uint32_t, std::vector<Ptr<QbbNetDevice>> > > switchToSwitch;

std::map<uint32_t, uint32_t> switchNumToId;
std::map<uint32_t, uint32_t> switchIdToNum;
std::map<uint32_t, NetDeviceContainer> switchUp;
std::map<uint32_t, NetDeviceContainer> switchDown;
//NetDeviceContainer switchUp[switch_num];
std::map<uint32_t, NetDeviceContainer> sourceNodes;

uint64_t nic_rate;

uint64_t maxRtt, maxBdp;

struct Interface {
    uint32_t idx;
    bool up;
    uint64_t delay;
    uint64_t bw;

    Interface() : idx(0), up(false) {}
};
map<Ptr<Node>, map<Ptr<Node>, Interface> > nbr2if;
// Mapping destination to next hop for each node: <node, <dest, <nexthop0, ...> > >
map<Ptr<Node>, map<Ptr<Node>, vector<Ptr<Node> > > > nextHop;
map<Ptr<Node>, map<Ptr<Node>, uint64_t> > pairDelay;
map<Ptr<Node>, map<Ptr<Node>, uint64_t> > pairTxDelay;
map<uint32_t, map<uint32_t, uint64_t> > pairBw;
map<Ptr<Node>, map<Ptr<Node>, uint64_t> > pairBdp;
map<uint32_t, map<uint32_t, uint64_t> > pairRtt;

void CalculateRoute(Ptr<Node> host) {
    // queue for the BFS.
    vector<Ptr<Node> > q;
    // Distance from the host to each node.
    map<Ptr<Node>, int> dis;
    map<Ptr<Node>, uint64_t> delay;
    map<Ptr<Node>, uint64_t> txDelay;
    map<Ptr<Node>, uint64_t> bw;
    // init BFS.
    q.push_back(host);
    dis[host] = 0;
    delay[host] = 0;
    txDelay[host] = 0;
    bw[host] = 0xfffffffffffffffflu;
    // BFS.
    for (int i = 0; i < (int)q.size(); i++) {
        Ptr<Node> now = q[i];
        int d = dis[now];
        for (auto it = nbr2if[now].begin(); it != nbr2if[now].end(); it++) {
            // skip down link
            if (!it->second.up)
                continue;
            Ptr<Node> next = it->first;
            // If 'next' have not been visited.
            if (dis.find(next) == dis.end()) {
                dis[next] = d + 1;
                delay[next] = delay[now] + it->second.delay;
                txDelay[next] = txDelay[now] + packet_payload_size * 1000000000lu * 8 / it->second.bw;
                bw[next] = std::min(bw[now], it->second.bw);
                // we only enqueue switch, because we do not want packets to go through host as middle point
                if (next->GetNodeType())
                    q.push_back(next);
            }
            // if 'now' is on the shortest path from 'next' to 'host'.
            if (d + 1 == dis[next]) {
                nextHop[next][host].push_back(now);
            }
        }
    }
    for (auto it : delay)
        pairDelay[it.first][host] = it.second;
    for (auto it : txDelay)
        pairTxDelay[it.first][host] = it.second;
    for (auto it : bw)
        pairBw[it.first->GetId()][host->GetId()] = it.second;
}

void CalculateRoutes(NodeContainer &n) {
    for (int i = 0; i < (int)n.GetN(); i++) {
        Ptr<Node> node = n.Get(i);
        if (node->GetNodeType() == 0)
            CalculateRoute(node);
    }
}

void SetRoutingEntries() {
    // For each node.
    for (auto i = nextHop.begin(); i != nextHop.end(); i++) {
        Ptr<Node> node = i->first;
        auto &table = i->second;
        for (auto j = table.begin(); j != table.end(); j++) {
            // The destination node.
            Ptr<Node> dst = j->first;
            // The IP address of the dst.
            Ipv4Address dstAddr = dst->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            // The next hops towards the dst.
            vector<Ptr<Node> > nexts = j->second;
            for (int k = 0; k < (int)nexts.size(); k++) {
                Ptr<Node> next = nexts[k];
                uint32_t interface = nbr2if[node][next].idx;
                DynamicCast<SwitchNode>(node)->AddTableEntry(dstAddr, interface);
            }
        }
    }
}

Ipv4Address node_id_to_ip(uint32_t id) {
    return Ipv4Address(0x0b000001 + ((id / 256) * 0x00010000) + ((id % 256) * 0x00000100));
}

/**
 * Topology:
 * Node1 ---> Node2
 *
 * Node1 sends CBR flow to Node2.
 */

int main(int argc, char *argv[]) {
    LogComponentEnable("GENERIC_SIMULATION", LOG_LEVEL_INFO);

    std::string AppPacketRate("10Mbps");
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(AppPacketRate));

    NS_LOG_INFO("Create nodes.");

    Ptr<SwitchNode> sw1 = CreateObject<SwitchNode>();
    sw1->SetNodeType(0);
    nodes.Add(sw1);
    Ptr<SwitchNode> sw2 = CreateObject<SwitchNode>();
    sw2->SetNodeType(0);
    nodes.Add(sw2);

    InternetStackHelper internet;
    Ipv4GlobalRoutingHelper globalRoutingHelper;
    internet.SetRoutingHelper(globalRoutingHelper);
    internet.Install(nodes);

    NS_LOG_INFO("Create channels.");

    QbbHelper qbb;
    Ipv4AddressHelper ipv4Helper;
    Ptr<Node> src = nodes.Get(0), dst = nodes.Get(1);

    std::string data_rate = "25000000000.0", link_daley = "1us";
    qbb.SetDeviceAttribute("DataRate", StringValue(data_rate));
    qbb.SetChannelAttribute("Delay", StringValue(link_daley));
    qbb.SetFlowChannelAttribute("DataRate", StringValue(AppPacketRate));

    NetDeviceContainer d = qbb.Install(src, dst);
    Ptr<Ipv4> ipv4Src = src->GetObject<Ipv4>();
    ipv4Src->AddInterface(d.Get(0));
    ipv4Src->AddAddress(1, Ipv4InterfaceAddress(node_id_to_ip(0), Ipv4Mask(0xff000000)));
    std::cout << "Src IP : " << node_id_to_ip(0) << std::endl;

    Ptr<Ipv4> ipv4Dst = dst->GetObject<Ipv4>();
    ipv4Dst->AddInterface(d.Get(1));
    ipv4Dst->AddAddress(1, Ipv4InterfaceAddress(node_id_to_ip(1), Ipv4Mask(0xff000000)));
    std::cout << "Dst IP : " << node_id_to_ip(1) << std::endl;

    nbr2if[src][dst].idx = DynamicCast<QbbNetDevice>(d.Get(0))->GetIfIndex();
    nbr2if[src][dst].up = true;
    nbr2if[src][dst].delay = DynamicCast<QbbChannel>(DynamicCast<QbbNetDevice>(d.Get(0))->GetChannel())->GetDelay().GetTimeStep();
    nbr2if[src][dst].bw = DynamicCast<QbbNetDevice>(d.Get(0))->GetDataRate().GetBitRate();
    nbr2if[dst][src].idx = DynamicCast<QbbNetDevice>(d.Get(1))->GetIfIndex();
    nbr2if[dst][src].up = true;
    nbr2if[dst][src].delay = DynamicCast<QbbChannel>(DynamicCast<QbbNetDevice>(d.Get(1))->GetChannel())->GetDelay().GetTimeStep();
    nbr2if[dst][src].bw = DynamicCast<QbbNetDevice>(d.Get(1))->GetDataRate().GetBitRate();

    ipv4Helper.SetBase("10.1.1.0", "255.255.255.0");
    ipv4Helper.Assign(d);

    CalculateRoutes(nodes);
    SetRoutingEntries();

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("Setup CBR Traffic Sources.");

    Ipv4InterfaceAddress ipv4DstIntAddr = ipv4Dst->GetAddress(1, 0);
    Ipv4Address ipDstAddr = ipv4DstIntAddr.GetLocal();
    u_int16_t port = 9;

    FlowOnOffHelper flowOnoff(
        "ns3::UdpSocketFactory",
        InetSocketAddress(ipDstAddr, port));
    flowOnoff.SetConstantRate(DataRate(AppPacketRate));
    ApplicationContainer apps = flowOnoff.Install(src);
    double AppStartTime = 1.0001;
    double AppStopTime = 2.0001;
    double SimTime = 2.5;

    apps.Start(Seconds(AppStartTime));
    apps.Stop(Seconds(AppStopTime));

    Simulator::Stop(Seconds(SimTime));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}