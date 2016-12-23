#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"


using namespace ns3;


//NS_LOG_COMPONENT_DEFINE ("TcpExample");


int main(int argc, char **argv)
{
	//setting the random seed for the start time
    RngSeedManager::SetSeed (11223344);
    Ptr<UniformRandomVariable> U = CreateObject<UniformRandomVariable> ();
    U->SetAttribute ("Stream", IntegerValue (6110));
    U->SetAttribute ("Min", DoubleValue (0.0));
    U->SetAttribute ("Max", DoubleValue (0.1));

    uint32_t nFlows = 10;
    uint32_t queueSize = 64000;
    uint32_t windowSize = 2000;
    uint32_t segSize = 512;
    

    CommandLine cmd;
    cmd.AddValue("queueSize","queue size",queueSize);
    cmd.AddValue("windowSize","window size",windowSize);
    cmd.AddValue("segSize","segment size",segSize);
    cmd.AddValue("nFlows","nflows",nFlows);
    cmd.Parse(argc,argv);

    double start[nFlows];

    for(unsigned int i = 0;i < nFlows; i++){
        start[i] = U->GetValue();
        //cout<<"Start time: "<<start[i]<<endl; //printing out start time so as to match the TA's output
    }

    uint32_t maxBytes = 0;
    //setting values to attributes
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (segSize));
    Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue (windowSize));
    Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue (false));
    
   // LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);
    //LogComponentEnable("PointToPointDumbbellHelper", LOG_LEVEL_INFO);


    
    // Setting up network topology
    // It is a dumbbell network with two leaves on each side
    PointToPointHelper PointToPointRouter;
    PointToPointHelper PointToPointLeaf;
	
	//creation of the bottleneck in the network
    PointToPointRouter.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    PointToPointRouter.SetChannelAttribute("Delay", StringValue("20ms"));
    PointToPointRouter.SetQueue("ns3::DropTailQueue","MaxBytes",UintegerValue(queueSize));
    PointToPointRouter.SetQueue("ns3::DropTailQueue","Mode",StringValue("QUEUE_MODE_BYTES"));

	//creating the leaf network
    PointToPointLeaf.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    PointToPointLeaf.SetChannelAttribute("Delay", StringValue("10ms"));

    PointToPointDumbbellHelper dumbbell(nFlows, PointToPointLeaf, nFlows, PointToPointLeaf, PointToPointRouter);
    
    InternetStackHelper stack;
    dumbbell.InstallStack(stack);

    dumbbell.AssignIpv4Addresses(Ipv4AddressHelper("10.0.1.0", "255.255.255.0"),
                                      Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                                      Ipv4AddressHelper("10.1.1.0", "255.255.255.0"));
    
   
   //Populate the routing tables.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();   
    
    // Setting up applications
    ApplicationContainer clientApps[nFlows]; // sources
    ApplicationContainer serverApps[nFlows]; //sinks

    for(unsigned int i = 0;i < nFlows; i++){
     BulkSendHelper clientHelper ("ns3::TcpSocketFactory", InetSocketAddress (dumbbell.GetRightIpv4Address(i), 9)); 
        clientHelper.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
        clientApps[i] = clientHelper.Install (dumbbell.GetLeft(i));
        clientApps[i].Start (Seconds (start[i])); //random start time	
        clientApps[i].Stop (Seconds (10.0));	//stop time
    }

    
    


    for(unsigned int i = 0;i < nFlows; i++){
    
         PacketSinkHelper serverHelper("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(),9));

        AddressValue local(InetSocketAddress(dumbbell.GetRightIpv4Address(i), 9));
        serverHelper.SetAttribute("Local", local);
        serverApps[i].Add(serverHelper.Install(dumbbell.GetRight(i)));

        
        // starting the simulation
        serverApps[i].Start(Seconds (start[i])); //random start time
        serverApps[i].Stop(Seconds(10.0)); //end time
    }
    
   
    Simulator::Stop (Seconds (10.0));
    Simulator::Run();
    
    for(unsigned int i = 0;i < nFlows; i++){
        Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (serverApps[i].Get (0));    
        std::cout << "flow"<<" "<<i<<" "<<"windowSize"<<" "<<windowSize<<" "<<"queueSize"<<" "<<queueSize<<" "<<"segSize"<<" "<<segSize<<" "<<"goodput"<<" "<< (double)sink1->GetTotalRx () / (double) (10.0 - start[i])  << std::endl;
    }
    Simulator::Destroy();

  

    return 0;
}
