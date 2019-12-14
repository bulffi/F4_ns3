#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/lte-module.h"
#include <ns3/buildings-helper.h>
#include "vstomp.h"
using namespace ns3;

int main(int argc, char* argv[]){

    uint16_t numEnb = 2;
    uint16_t numUePerEnb = 3;
    Time simTime = MilliSeconds (1100);
    double distance = 60.0;

    CommandLine cmd;
    cmd.AddValue ("numEnb", "Number of eNodeBs", numEnb);
    cmd.AddValue ("numUePerEnb", "Number of UE for each eNodeB", numUePerEnb)
    cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
    cmd.AddValue ("distance", "Distance between eNBs [m]", distance);
    
    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults ();

    cmd.Parse (argc, argv);

    if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }
    
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
    lteHelper->SetEpcHelper (epcHelper);

    Ptr<Node> pgw = epcHelper->GetPgwNode ();

    // Create a single RemoteHost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (1);
    Ptr<Node> remoteHost = remoteHostContainer.Get (0);
    InternetStackHelper internet;
    internet.Install (remoteHostContainer);

    // Create the Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
    // interface 0 is localhost, 1 is the p2p device
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create (numEnb);
    ueNodes.Create (numEnb*numUePerEnb);

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (enbNodes);
    BuildingsHelper::Install (enbNodes);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (ueNodes);
    BuildingsHelper::Install (ueNodes);

    // Install LTE Devices to the nodes
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

    // Install the IP stack on the UEs
    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
    // Assign IP address to UEs, and install applications
    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
      {
        Ptr<Node> ueNode = ueNodes.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      }

    // Attach one UE per eNodeB
    for (uint16_t i = 0; i < numEnb; i++)
      {
        for (uint16_t j = 0; j < numUePerEnb; j++){
          lteHelper->Attach (ueLteDevs.Get(i*numUePerEnb+j), enbLteDevs.Get(i));
        }
        // side effect: the default EPS bearer will be activated
      }

    // 应用层
    // 需要的输入是一个已经安装了 TCP 协议的 server 以及它的 ns3::Ipv4Address 
    // 一个数组，包含一组已经安装了 TCP 协议的 client

    ClientWithMessages client1;
    client1.node = ueNodes.Get(1);
    std::vector<std::string> channels;
    channels.push_back("hhh");
    channels.push_back("xixi");
    std::vector<Message> messages;
    Message message1;
    message1.target="zzj";
    message1.content="say hello to myself";
    Message message2;
    message2.target="hhh";
    message2.content="try out my channel";
    messages.push_back(message1);
    messages.push_back(message2);
    client1.userName="zzj";
    client1.channelsTosub = channels;
    client1.messagesToSend = messages;
    // ClientWithMessages client2;
    // ClientWithMessages client3;
    std::vector<ClientWithMessages> clients;
    clients.push_back(client1);
    vStompApplication application = vStompApplication(remoteHost,remoteHostAddr,clients);
    application.start();

    Simulator::Stop(Seconds(20));
    NS_LOG_INFO("The simulation begins");
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}

