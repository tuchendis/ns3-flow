// ---------- Header Includes -------------------------------------------------

#include "ns3/applications-module.h"
#include "ns3/assert.h"
#include "ns3/core-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace ns3;

// ---------- Prototypes ------------------------------------------------------

std::vector<std::vector<bool>> readNxNMatrix(std::string adj_mat_file_name);
std::vector<std::vector<double>> readCoordinatesFile(std::string node_coordinates_file_name);
void printCoordinateArray(const char* description, std::vector<std::vector<double>> coord_array);
void printMatrix(const char* description, std::vector<std::vector<bool>> array);

NS_LOG_COMPONENT_DEFINE("DumbellFlowTest");

std::ofstream flow_records;

Ptr<FlowMonitor> flowmon;
FlowMonitorHelper flowmonHelper;
std::map<Ipv4Address, uint32_t> rx_bytes;
Time lastRx = Seconds(0);

void CountRxBytes(Ptr<const Packet> pkt) {
//    std::cout << "have received " << pkt->GetSize() << " bytes." << std::endl;
    Ptr<Packet> copiedPacket = pkt->Copy();
    ns3::PppHeader pppHeader;
    Ipv4Header ipHeader;
    UdpHeader udpHeader;

    copiedPacket->PeekHeader(pppHeader);
//    NS_LOG_INFO("PPP Header: protocol=" << pppHeader.GetProtocol());

    copiedPacket->RemoveHeader(pppHeader);
    copiedPacket->PeekHeader(ipHeader);
//    NS_LOG_INFO("IPv4 Header: " << ipHeader.GetSource() << " -> " << ipHeader.GetDestination());

    copiedPacket->RemoveHeader(ipHeader);
    copiedPacket->PeekHeader(udpHeader);

//    NS_LOG_INFO("UDP Header: srcPort=" << udpHeader.GetSourcePort() << " dstPort=" << udpHeader.GetDestinationPort());
//    double interval = (Simulator::Now() - lastRx).GetSeconds();
//    NS_LOG_INFO("PacketSize: " << pkt->GetSize() << " Interval: " << Simulator::Now() - lastRx);
//    lastRx = Simulator::Now();
//    NS_LOG_INFO("Rate: " << pkt->GetSize() / interval);

    Ipv4Address srcIp = ipHeader.GetSource();

    if (rx_bytes.find(srcIp) == rx_bytes.end()) {
        rx_bytes[srcIp] = 0;
    }
    rx_bytes[srcIp] += pkt->GetSize();
    flow_records << srcIp << "," << rx_bytes[srcIp] << "," << Simulator::Now() << std::endl;
}

int main(int argc, char* argv[])
{
    // Simulation Parameters
    double SimTime = 10.00;
    double SinkStartTime = 1.0001;
    double SinkStopTime = 9.90001;
    double AppStartTime = 2.0001;
    double AppStopTime = 9.80001;

    std::string AppPacketRate("40Kbps");
    Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("1000"));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(AppPacketRate));
    Config::SetDefault("ns3::BulkSendApplication::SendSize", StringValue("1024"));
    std::string LinkRate("1Mbps");
    std::string LinkDelay("2ms");

    std::string tr_name("n-node-dumbell.tr");
    std::string flow_name("n-node-dumble.xml");
    std::string flow_records_name("flow_records.csv");

    std::string adj_mat_file_name("/home/wkli/ns-3-dev/examples/flow/adjacency_matrix.txt");

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    LogComponentEnable("DumbellFlowTest", LOG_LEVEL_INFO);

    // Read Adjacency Matrix
    std::vector<std::vector<bool>> Adj_Matrix = readNxNMatrix(adj_mat_file_name);
    int n_nodes = Adj_Matrix.size();

    // Create Nodes
    NS_LOG_INFO("Create Nodes.");
    NodeContainer nodes;
    nodes.Create(n_nodes);

    // Set up P2P Link attributes
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(LinkRate));
    p2p.SetChannelAttribute("Delay", StringValue(LinkDelay));

    // Install Internet stack on nodes
    NS_LOG_INFO("Install Internet Stack to Nodes.");
    InternetStackHelper internet;
    internet.Install(NodeContainer::GetGlobal());

    // Assign IP Addresses to nodes
    NS_LOG_INFO("Assign Address to Nodes.");
    Ipv4AddressHelper ipv4_n;
    ipv4_n.SetBase("10.0.0.0", "255.255.255.252");

    // Create Links Between Nodes
    NS_LOG_INFO("Create Links Between Nodes.");
    uint32_t linkCount = 0;
    for (int i = 0; i < n_nodes; i++) {
        for (int j = 0; j < n_nodes; j++) {
            if (Adj_Matrix[i][j]) {
                NodeContainer n_links = NodeContainer(nodes.Get(i), nodes.Get(j));
                NetDeviceContainer n_devs = p2p.Install(n_links);
                if (j == 3) {
//                    n_devs.Get(0)->TraceConnectWithoutContext("MacRx", MakeBoundCallback(&CountRxBytes));
                    n_devs.Get(1)->TraceConnectWithoutContext("MacRx", MakeCallback(&CountRxBytes));
//                    n_devs.Get(0)->SetMtu(1500);
//                    n_devs.Get(1)->SetMtu(1500);
                }

                ipv4_n.Assign(n_devs);
                ipv4_n.NewNetwork();
                linkCount++;
                NS_LOG_INFO("matrix element [" << i << "][" << j << "] is 1");
            }
        }
    }
    NS_LOG_INFO("Number of links: " << linkCount);

    // Enable Global Routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Setup Packet Sink on the receiver node
    uint16_t port = 9;
    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
//    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer app_sink = sink.Install(nodes.Get(3));
    app_sink.Start(Seconds(SinkStartTime));
    app_sink.Stop(Seconds(SinkStopTime));

//    UdpEchoServerHelper server(port);
//    ApplicationContainer apps = server.Install(nodes.Get(3));
//    apps.Start(Seconds(SinkStartTime));
//    apps.Stop(Seconds(SinkStopTime));

    //
    // Create a UdpEchoClient application to send UDP datagrams from node zero to
    // node one.
    //
//    uint32_t packetSize = 1024;
//    uint32_t maxPacketCount = 1000;
//    Time interPacketInterval = Seconds(0.1);

    // Setup OnOff application for sending traffic
    for (int i = 0; i < 1; i++) {
        if (i == 2 || i == 3) continue;
        Ptr<Node> n = nodes.Get(3);
        Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
        Ipv4InterfaceAddress ipv4_int_addr = ipv4->GetAddress(1, 0);
        Ipv4Address ip_addr = ipv4_int_addr.GetLocal();

        // OnOff
        OnOffHelper onoff("ns3::UdpSocketFactory", InetSocketAddress(ip_addr, port));
        onoff.SetConstantRate(DataRate(AppPacketRate));
        onoff.SetAttribute("PacketSize", UintegerValue(1024));
        ApplicationContainer apps = onoff.Install(nodes.Get(i));

        // Bulk
//        BulkSendHelper bulk_sender("ns3::TcpSocketFactory", InetSocketAddress(ip_addr, port));
//        bulk_sender.SetAttribute("MaxBytes", UintegerValue(0));
//        bulk_sender.SetAttribute("SendSize", UintegerValue(1024));
//        ApplicationContainer apps = bulk_sender.Install(nodes.Get(i));

        // UdpEcho
//        UdpEchoClientHelper client(ip_addr, port);
//        client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
//        client.SetAttribute("Interval", TimeValue(interPacketInterval));
//        client.SetAttribute("PacketSize", UintegerValue(packetSize));
//        apps = client.Install(nodes.Get(i));

        apps.Start(Seconds(AppStartTime + 0.1 * i));
        apps.Stop(Seconds(AppStopTime));
    }

    // Enable Tracing
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream(tr_name));

    flowmon = flowmonHelper.InstallAll();

    flow_records.open(flow_records_name, std::ios::out);
    if (flow_records.fail())
    {
        NS_FATAL_ERROR("File " << flow_records_name << " not found");
    }

    // Run the simulation
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(SimTime));
    Simulator::Run();
    Simulator::Destroy();

    flow_records.close();

    return 0;
}


// ---------- Function Definitions -------------------------------------------

std::vector<std::vector<bool>>
readNxNMatrix(std::string adj_mat_file_name)
{
    std::ifstream adj_mat_file;
    adj_mat_file.open(adj_mat_file_name, std::ios::in);
    if (adj_mat_file.fail())
    {
        NS_FATAL_ERROR("File " << adj_mat_file_name << " not found");
    }
    std::vector<std::vector<bool>> array;
    int i = 0;
    int n_nodes = 0;

    while (!adj_mat_file.eof())
    {
        std::string line;
        getline(adj_mat_file, line);
        if (line.empty())
        {
            NS_LOG_WARN("WARNING: Ignoring blank row in the array: " << i);
            break;
        }

        std::istringstream iss(line);
        bool element;
        std::vector<bool> row;
        int j = 0;

        while (iss >> element)
        {
            row.push_back(element);
            j++;
        }

        if (i == 0)
        {
            n_nodes = j;
        }

        if (j != n_nodes)
        {
            NS_LOG_ERROR("ERROR: Number of elements in line "
                         << i << ": " << j
                         << " not equal to number of elements in line 0: " << n_nodes);
            NS_FATAL_ERROR("ERROR: The number of rows is not equal to the number of columns! in "
                           "the adjacency matrix");
        }
        else
        {
            array.push_back(row);
        }
        i++;
    }

    if (i != n_nodes)
    {
        NS_LOG_ERROR("There are " << i << " rows and " << n_nodes << " columns.");
        NS_FATAL_ERROR("ERROR: The number of rows is not equal to the number of columns! in the "
                       "adjacency matrix");
    }

    adj_mat_file.close();
    return array;
}

std::vector<std::vector<double>>
readCoordinatesFile(std::string node_coordinates_file_name)
{
    std::ifstream node_coordinates_file;
    node_coordinates_file.open(node_coordinates_file_name, std::ios::in);
    if (node_coordinates_file.fail())
    {
        NS_FATAL_ERROR("File " << node_coordinates_file_name << " not found");
    }
    std::vector<std::vector<double>> coord_array;
    int m = 0;

    while (!node_coordinates_file.eof())
    {
        std::string line;
        getline(node_coordinates_file, line);

        if (line.empty())
        {
            NS_LOG_WARN("WARNING: Ignoring blank row: " << m);
            break;
        }

        std::istringstream iss(line);
        double coordinate;
        std::vector<double> row;
        int n = 0;
        while (iss >> coordinate)
        {
            row.push_back(coordinate);
            n++;
        }

        if (n != 2)
        {
            NS_LOG_ERROR("ERROR: Number of elements at line#"
                         << m << " is " << n
                         << " which is not equal to 2 for node coordinates file");
            exit(1);
        }

        else
        {
            coord_array.push_back(row);
        }
        m++;
    }
    node_coordinates_file.close();
    return coord_array;
}

void
printMatrix(const char* description, std::vector<std::vector<bool>> array)
{
    std::cout << "**** Start " << description << "********" << std::endl;
    for (size_t m = 0; m < array.size(); m++)
    {
        for (size_t n = 0; n < array[m].size(); n++)
        {
            std::cout << array[m][n] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << "**** End " << description << "********" << std::endl;
}

void
printCoordinateArray(const char* description, std::vector<std::vector<double>> coord_array)
{
    std::cout << "**** Start " << description << "********" << std::endl;
    for (size_t m = 0; m < coord_array.size(); m++)
    {
        for (size_t n = 0; n < coord_array[m].size(); n++)
        {
            std::cout << coord_array[m][n] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << "**** End " << description << "********" << std::endl;
}

// ---------- End of Function Definitions ------------------------------------
