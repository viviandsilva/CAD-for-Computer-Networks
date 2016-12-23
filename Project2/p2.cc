#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

// Network topology
// 
//                       n5                    n6 
//                     (UDP source)         (UDP source)
//                        |                     |
//                        |  6 Mbps             |  10 Mbps
//                        |  10 ms              |  5 ms
//                        |                     | 
//       n0 ------------- n1-------------------n2------------------n3---------------n4
// (TCP        5 Mbps     |       1 Mbps        |       1 Mbps            5 Mbps     (TCP and UDP sinks)
//  source)    10 ms      |       20 ms         |       20 ms             10 ms
//            (variable)  |                     |                       (variable)
//                        | 6 Mbps              | 5 Mbps
//                        | 10 ms               | 10 ms
//                        n7                    n8
//                     (TCP source)          (TCP source)
//                  


// - Flow from n0 to n4 using BulkSendApplication.
// - Flow from n7 to n4 using BulkSendApplication.
// - Flow from n8 to n4 using BulkSendApplication.
// - Flow from n5 to n4 using OnOffApplication.
// - Flow from n6 to n4 using OnOffApplication.
// - node n1, n2 and n3 are routers




using namespace ns3;
using namespace std;

//NS_LOG_COMPONENT_DEFINE ("REDExample");



int main(int argc, char **argv)
{

//default values
    uint32_t maxBytes = 0;
    uint32_t queueSize = 64000;
    uint32_t windowSize = 64000;
    uint32_t pktSize = 128;
    uint32_t qlen =480*pktSize ;  
    double minTh = 5;
    double maxTh = 15;
    double qw = 1.0/128.0;
    double maxP =0;
    string Rate = "0.5Mbps";        //setting data rate for UDP sources, this will affect the traffic in the network
    string RTT = "10ms";            //setting delay for a few links, this will affect the round trip time
    string red_dt = "RED";
    string bottleNeckLinkBw = "1Mbps";
    string bottleNeckLinkDelay = "20ms";
    
    
//command line arguments    
    CommandLine cmd;
    cmd.AddValue("red_dt","red_droptail",red_dt);
    cmd.AddValue("RTT","RTT",RTT);
    cmd.AddValue("queueSize","queue size",queueSize);
    cmd.AddValue("windowSize","window size",windowSize);
    cmd.AddValue("minTh","minTh",minTh);
    cmd.AddValue("maxTh","maxTh",maxTh);
    cmd.AddValue("qw","qw",qw);
    cmd.AddValue("maxP","maxP",maxP);
    cmd.AddValue("Rate","Rate",Rate);
    cmd.AddValue("qlen","qlen", qlen);
    cmd.Parse(argc,argv);

    
//Options
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
    Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue (windowSize));
    Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue (false));
    Config::SetDefault("ns3::RedQueue::LInterm",DoubleValue(maxP));
    Config::SetDefault("ns3::RedQueue::QueueLimit",UintegerValue(qlen));


    
//Creating network topology
    NodeContainer p2pNodes;
    p2pNodes.Create (9);

    PointToPointHelper PointToPointRouter; //Helper for the Routers
    PointToPointHelper PointToPointNode01;
    PointToPointHelper PointToPointNode34;
    PointToPointHelper PointToPointNode51;
    PointToPointHelper PointToPointNode62;
    PointToPointHelper PointToPointNode71;
    PointToPointHelper PointToPointNode82;
    
//Setting Attributes for RED or DropTail, depending on the input for red_dt   
    if ((red_dt != "RED") && (red_dt != "DT")){
      NS_ABORT_MSG ("Invalid queue type: Use --red_dt=RED or --red_dt=DT");
    }

    if(red_dt == "DT"){
        PointToPointRouter.SetDeviceAttribute("DataRate", StringValue(bottleNeckLinkBw));
        PointToPointRouter.SetChannelAttribute("Delay", StringValue(bottleNeckLinkDelay));
        PointToPointRouter.SetQueue("ns3::DropTailQueue","MaxBytes",UintegerValue(queueSize));
        PointToPointRouter.SetQueue("ns3::DropTailQueue","Mode",StringValue("QUEUE_MODE_BYTES"));
        std::cout<<"queue Size:"<<" "<<queueSize<<" "<<"window Size:"<<" "<<windowSize<<" "<<"RTT:"<<" "<<RTT<<" "<<"DataRate:"<<" "<<Rate<<" ";
    } else if (red_dt == "RED"){
        minTh *= pktSize; 
        maxTh *= pktSize;
        PointToPointRouter.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckLinkBw));
        PointToPointRouter.SetChannelAttribute ("Delay", StringValue (bottleNeckLinkDelay));
        PointToPointRouter.SetQueue("ns3::RedQueue","QueueLimit",UintegerValue(qlen));
        PointToPointRouter.SetQueue("ns3::RedQueue","QW",DoubleValue(qw));
        PointToPointRouter.SetQueue("ns3::RedQueue","Mode",StringValue("QUEUE_MODE_BYTES"));
        PointToPointRouter.SetQueue ("ns3::RedQueue","MinTh", DoubleValue (minTh),"MaxTh", DoubleValue (maxTh),"LinkBandwidth", StringValue (bottleNeckLinkBw),"LinkDelay", StringValue (bottleNeckLinkDelay));
        std::cout <<"MinTh:"<<" "<<minTh<<" "<<"MaxTh:"<<" "<<maxTh<<" "<<"Queue Length"<<" "<<qlen<<" "<<"maxP"<<" "<<maxP<<" "<<"RTT:"<<" "<<RTT<<" "<<"DataRate:"<<" "<<Rate<<" ";
        
    }

    PointToPointNode01.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    PointToPointNode01.SetChannelAttribute("Delay", StringValue(RTT));

    PointToPointNode34.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    PointToPointNode34.SetChannelAttribute("Delay", StringValue(RTT));

    PointToPointNode51.SetDeviceAttribute("DataRate", StringValue("6Mbps"));
    PointToPointNode51.SetChannelAttribute("Delay", StringValue("10ms"));

    PointToPointNode62.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    PointToPointNode62.SetChannelAttribute("Delay", StringValue("5ms"));


    PointToPointNode71.SetDeviceAttribute("DataRate", StringValue("6Mbps"));
    PointToPointNode71.SetChannelAttribute("Delay", StringValue("10ms"));

    PointToPointNode82.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    PointToPointNode82.SetChannelAttribute("Delay", StringValue("10ms"));





    //Defines a NodeContainer object that holds Devices
    NodeContainer net01 (p2pNodes.Get(0), p2pNodes.Get(1));
    NodeContainer net12 (p2pNodes.Get(1), p2pNodes.Get(2)); 
    NodeContainer net23 (p2pNodes.Get(2), p2pNodes.Get(3)); 
    NodeContainer net34 (p2pNodes.Get(3), p2pNodes.Get(4));
    NodeContainer net51 (p2pNodes.Get(5),p2pNodes.Get(1));
    NodeContainer net62 (p2pNodes.Get(6),p2pNodes.Get(2));
    NodeContainer net71 (p2pNodes.Get(7),p2pNodes.Get(1));
    NodeContainer net82 (p2pNodes.Get(8),p2pNodes.Get(2));

    
    NetDeviceContainer nodeDevices01;
    nodeDevices01 = PointToPointNode01.Install(net01);

    NetDeviceContainer routerDevices12;
    routerDevices12 = PointToPointRouter.Install (net12);

    NetDeviceContainer routerDevices23;
    routerDevices23 = PointToPointRouter.Install (net23);

    NetDeviceContainer nodeDevices34;
    nodeDevices34 = PointToPointNode34.Install(net34);

    NetDeviceContainer nodeDevices51;
    nodeDevices51 = PointToPointNode51.Install(net51);

    NetDeviceContainer nodeDevices62;
    nodeDevices62 = PointToPointNode62.Install(net62);
    
    NetDeviceContainer nodeDevices71;
    nodeDevices71 = PointToPointNode71.Install(net71);
    
    NetDeviceContainer nodeDevices82;
    nodeDevices82 = PointToPointNode82.Install(net82);

    
    InternetStackHelper stack;
    stack.Install(p2pNodes);
    

   // NS_LOG_INFO ("Assign IP Addresses.");

//Assigning IP addresses to interfaces
    Ipv4AddressHelper address1;
    address1.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces1;
    p2pInterfaces1 = address1.Assign (nodeDevices01);

    Ipv4AddressHelper address2;
    address2.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces2;
    p2pInterfaces2 = address2.Assign (routerDevices12);

    Ipv4AddressHelper address3;
    address3.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces3;
    p2pInterfaces3 = address3.Assign (routerDevices23);

    Ipv4AddressHelper address4;
    address4.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces4;
    p2pInterfaces4 = address4.Assign (nodeDevices34);

    Ipv4AddressHelper address5;
    address5.SetBase ("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces5;
    p2pInterfaces5 = address5.Assign (nodeDevices51);

    Ipv4AddressHelper address6;
    address6.SetBase ("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces6;
    p2pInterfaces6 = address6.Assign (nodeDevices62);
    
    Ipv4AddressHelper address7;
    address7.SetBase ("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces7;
    p2pInterfaces7 = address7.Assign (nodeDevices71);
    
    Ipv4AddressHelper address8;
    address8.SetBase ("10.1.8.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces8;
    p2pInterfaces8= address8.Assign (nodeDevices82);


    //Populate the routing tables.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();   
    
 

 
     uint16_t port = 53;  

//TCP source 1 on node 0 using BulkSendApplication
    BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (p2pInterfaces4.GetAddress (1), port));
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    source.SetAttribute ("SendSize", UintegerValue (pktSize));
    ApplicationContainer sourceApps = source.Install (p2pNodes.Get (0));
    sourceApps.Start (Seconds (1.0));
    sourceApps.Stop (Seconds (10.0));


//TCP source 2 on node 7 using BulkSendApplication
    BulkSendHelper source1 ("ns3::TcpSocketFactory",InetSocketAddress (p2pInterfaces4.GetAddress (1), port));
     // Set the amount of data to send in bytes.  Zero is unlimited.
    source1.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    source1.SetAttribute ("SendSize", UintegerValue (pktSize));
    ApplicationContainer sourceApps1 = source1.Install (p2pNodes.Get (7));
    sourceApps1.Start (Seconds (1.0));
    sourceApps1.Stop (Seconds (10.0)); 

//TCP source 3 on node 8 using BulkSendApplication
    BulkSendHelper source2 ("ns3::TcpSocketFactory",InetSocketAddress (p2pInterfaces4.GetAddress (1), port));
     // Set the amount of data to send in bytes.  Zero is unlimited.
    source2.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    source2.SetAttribute ("SendSize", UintegerValue (pktSize));
    ApplicationContainer sourceApps2 = source2.Install (p2pNodes.Get (8));
    sourceApps2.Start (Seconds (1.0));
    sourceApps2.Stop (Seconds (10.0));

//TCP sink on node 4 
    PacketSinkHelper sink ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApps = sink.Install (p2pNodes.Get (4));
    sinkApps.Start (Seconds (1.0));
    sinkApps.Stop (Seconds (10.0));
    
//UDP source 1 on node 5 using OnOffApplication  
    DataRate x(Rate);
    OnOffHelper clientHelper ("ns3::UdpSocketFactory", InetSocketAddress (p2pInterfaces4.GetAddress (1), port));
    clientHelper.SetConstantRate(x,pktSize);
    clientHelper.SetAttribute ("PacketSize", UintegerValue (pktSize));
    ApplicationContainer srcApps = clientHelper.Install (p2pNodes.Get (5));
    srcApps.Start (Seconds (1.0));
    srcApps.Stop (Seconds (10.0));
    
//UDP source 2 on node 6 using OnOffApplication
    OnOffHelper clientHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (p2pInterfaces4.GetAddress (1), port));
    clientHelper2.SetConstantRate(x,pktSize);
    clientHelper2.SetAttribute ("PacketSize", UintegerValue (pktSize));
    ApplicationContainer srcApps2 = clientHelper2.Install (p2pNodes.Get (6));
    srcApps2.Start (Seconds (1.0));
    srcApps2.Stop (Seconds (10.0));

//UDP sink on node 4    
    Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
    PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkLocalAddress);
    ApplicationContainer apps;
    apps.Add (packetSinkHelper.Install (p2pNodes.Get (4)));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));    

   
    Simulator::Stop (Seconds (10.0));
    Simulator::Run();
    
     Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));    
    std::cout<<"TCP Goodput:"<< (double)sink1->GetTotalRx () / (double)10.0<<" ";

    Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (apps.Get (0));    
    std::cout <<"UDP Goodput:"<< (double)sink2->GetTotalRx () / (double)10.0<< std::endl;


    Simulator::Destroy();

    return 0;
}
