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

std::vector<Ipv4Address> serverAddress;

/**
 * Topology:
 * Node1 ---> Node2
 *
 * Node1 sends CBR flow to Node2.
 */

int main(int argc, char *argv[]) {
    LogComponentEnable("GENERIC_SIMULATION", LOG_LEVEL_INFO);

    std::string AppPacketRate("20Mbps");
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(AppPacketRate));

    std::string topology_file("/home/wkli/ns3-datacenter/simulator/ns-3.39/examples/flow/topology.txt");
    std::ifstream topof;

    topof.open(topology_file.c_str());
    uint32_t node_num, switch_num, tors, link_num, trace_num;
    topof >> node_num >> switch_num >> tors >> link_num; // changed here. The previous order was node, switch, link // tors is not used. switch_num=tors for now.
    tors = switch_num;
    std::cout << node_num << " " << switch_num << " " << tors <<  " " << link_num << std::endl;

    NodeContainer serverNodes;
    NodeContainer torNodes;
    NodeContainer spineNodes;
    NodeContainer switchNodes;
    NodeContainer allNodes;

    std::vector<uint32_t> node_type(node_num, 0);
    std::cout << "switch_num " << switch_num << std::endl;
    for (uint32_t i = 0; i < switch_num; i++) {
        uint32_t sid;
        topof >> sid;
        std::cout << "sid " << sid << std::endl;
        switchNumToId[i] = sid;
        switchIdToNum[sid] = i;
        if (i < tors) {
            node_type[sid] = 1;
        }
        else
            node_type[sid] = 2;
    }

    for (uint32_t i = 0; i < node_num; i++) {
        Ptr<SwitchNode> sw = CreateObject<SwitchNode>();
        nodes.Add(sw);
        switchNodes.Add(sw);
        sw->SetNodeType(node_type[i]);
        allNodes.Add(sw);
    }

    NS_LOG_INFO("Create nodes.");

    InternetStackHelper internet;
    Ipv4GlobalRoutingHelper globalRoutingHelper;
    internet.SetRoutingHelper (globalRoutingHelper);
    internet.Install(nodes);

    //
    // Assign IP to each server
    //
    for (uint32_t i = 0; i < node_num; i++) {
        if (nodes.Get(i)->GetNodeType() == 0) { // is server
            serverAddress.resize(i + 1);
            serverAddress[i] = node_id_to_ip(i);
        }
    }

    NS_LOG_INFO("Create channels.");

    //
    // Explicitly create the channels required by the topology.
    //

    QbbHelper qbb;
    Ipv4AddressHelper ipv4;
    for (uint32_t i = 0; i < link_num; i++)
    {
        uint32_t src, dst;
        std::string data_rate, link_delay;
        topof >> src >> dst >> data_rate >> link_delay;

        std::cout << src << " " << dst << " " << nodes.GetN() << " " << data_rate << " " << link_delay << " " << std::endl;
        Ptr<Node> snode = nodes.Get(src), dnode = nodes.Get(dst);

        qbb.SetDeviceAttribute("DataRate", StringValue(data_rate));
        qbb.SetChannelAttribute("Delay", StringValue(link_delay));
        qbb.SetFlowChannelAttribute("DataRate", StringValue("20Mbps"));
        qbb.SetFlowChannelAttribute("Delay", StringValue(link_delay));

        fflush(stdout);

        // Assign server IP
        // Note: this should be before the automatic assignment below (ipv4.Assign(d)),
        // because we want our IP to be the primary IP (first in the IP address list),
        // so that the global routing is based on our IP
        NetDeviceContainer d = qbb.Install(snode, dnode);
        if (snode->GetNodeType() == 0) {
            Ptr<Ipv4> ipv4 = snode->GetObject<Ipv4>();
            ipv4->AddInterface(d.Get(0));
            ipv4->AddAddress(1, Ipv4InterfaceAddress(serverAddress[src], Ipv4Mask(0xff000000)));
        }
        if (dnode->GetNodeType() == 0) {
            Ptr<Ipv4> ipv4 = dnode->GetObject<Ipv4>();
            ipv4->AddInterface(d.Get(1));
            ipv4->AddAddress(1, Ipv4InterfaceAddress(serverAddress[dst], Ipv4Mask(0xff000000)));
        }

        if (!snode->GetNodeType()) {
            sourceNodes[src].Add(DynamicCast<QbbNetDevice>(d.Get(0)));
        }

        if (!snode->GetNodeType() && dnode->GetNodeType()) {
            switchDown[switchIdToNum[dst]].Add(DynamicCast<QbbNetDevice>(d.Get(1)));
        }


        if (snode->GetNodeType() && dnode->GetNodeType()) {
            switchToSwitchInterfaces.Add(d);
            switchUp[switchIdToNum[src]].Add(DynamicCast<QbbNetDevice>(d.Get(0)));
            switchUp[switchIdToNum[dst]].Add(DynamicCast<QbbNetDevice>(d.Get(1)));
            switchToSwitch[src][dst].push_back(DynamicCast<QbbNetDevice>(d.Get(0)));
            switchToSwitch[src][dst].push_back(DynamicCast<QbbNetDevice>(d.Get(1)));
        }

        // used to create a graph of the topology
        nbr2if[snode][dnode].idx = DynamicCast<QbbNetDevice>(d.Get(0))->GetIfIndex();
        nbr2if[snode][dnode].up = true;
        nbr2if[snode][dnode].delay = DynamicCast<QbbChannel>(DynamicCast<QbbNetDevice>(d.Get(0))->GetChannel())->GetDelay().GetTimeStep();
        nbr2if[snode][dnode].bw = DynamicCast<QbbNetDevice>(d.Get(0))->GetDataRate().GetBitRate();
        nbr2if[dnode][snode].idx = DynamicCast<QbbNetDevice>(d.Get(1))->GetIfIndex();
        nbr2if[dnode][snode].up = true;
        nbr2if[dnode][snode].delay = DynamicCast<QbbChannel>(DynamicCast<QbbNetDevice>(d.Get(1))->GetChannel())->GetDelay().GetTimeStep();
        nbr2if[dnode][snode].bw = DynamicCast<QbbNetDevice>(d.Get(1))->GetDataRate().GetBitRate();

        // This is just to set up the connectivity between nodes. The IP addresses are useless
        // char ipstring[16];
        std::stringstream ipstring;
        ipstring << "10." << i / 254 + 1 << "." << i % 254 + 1 << ".0";
        // sprintf(ipstring, "10.%d.%d.0", i / 254 + 1, i % 254 + 1);
        ipv4.SetBase(ipstring.str().c_str(), "255.255.255.0");
        ipv4.Assign(d);

        // setup PFC trace
        // DynamicCast<QbbNetDevice>(d.Get(0))->TraceConnectWithoutContext("QbbPfc", MakeBoundCallback (&get_pfc, pfc_file, DynamicCast<QbbNetDevice>(d.Get(0))));
        // DynamicCast<QbbNetDevice>(d.Get(1))->TraceConnectWithoutContext("QbbPfc", MakeBoundCallback (&get_pfc, pfc_file, DynamicCast<QbbNetDevice>(d.Get(1))));
    }

    CalculateRoutes(nodes);
    SetRoutingEntries();

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("Setup CBR Traffic Sources.");

    Ptr<Node> dst = nodes.Get(2);
    Ptr<Ipv4> ipv4Dst = dst->GetObject<Ipv4>();
    Ipv4InterfaceAddress ipv4DstIntAddr = ipv4Dst->GetAddress(1, 0);
    Ipv4Address ipSrcAddr = nodes.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    Ipv4Address ipDstAddr = ipv4DstIntAddr.GetLocal();
    u_int16_t port = 9;

    Ptr<Node> src = nodes.Get(0);
    FlowOnOffHelper flowOnoff(
        "ns3::UdpSocketFactory",
        InetSocketAddress(ipSrcAddr, port),
        InetSocketAddress(ipDstAddr, port));
    flowOnoff.SetConstantRate(DataRate(AppPacketRate));
    ApplicationContainer apps = flowOnoff.Install(src);
    double AppStartTime = 1.0001;
    double AppStopTime = 2.0001;
    double SimTime = 2.5;

    apps.Start(Seconds(AppStartTime));
    apps.Stop(Seconds(AppStopTime));

    Ptr<Node> src2 = nodes.Get(3);
    Ipv4Address ipSrc2Addr = nodes.Get(3)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    FlowOnOffHelper flowOnoff2(
        "ns3::UdpSocketFactory",
        InetSocketAddress(ipSrc2Addr, port),
        InetSocketAddress(ipDstAddr, port));
    flowOnoff2.SetConstantRate(DataRate(AppPacketRate));
    ApplicationContainer apps2 = flowOnoff2.Install(src2);

    apps2.Start(Seconds(AppStartTime + 0.5));
    apps2.Stop(Seconds(AppStopTime));

    Simulator::Stop(Seconds(SimTime));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}