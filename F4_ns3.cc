#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store-module.h"
#include "vstomp.h"
using namespace ns3;

int main(int argc, char* argv[]){

    uint16_t numEnb = 3;
    uint16_t numUePerEnb = 4;
    double distance = 10000.0;
    uint16_t numClient = 3;
    bool tracing = true;

    CommandLine cmd;
    cmd.AddValue ("numEnb", "Number of eNodeBs", numEnb);
    cmd.AddValue ("numUePerEnb", "Number of UE for each eNodeB", numUePerEnb);
    cmd.AddValue ("numClient", "Number of clients of Vstomp protocol", numClient);
    cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

    cmd.Parse (argc, argv);

    LogComponentEnable("F4ComputerNetwork",LOG_LEVEL_INFO);

    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults ();

    cmd.Parse(argc, argv);
    
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
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create (numEnb);
    ueNodes.Create (numEnb*numUePerEnb);

    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(enbNodes);

    for (uint16_t i = 0; i < numEnb; i++)
    {
      for(uint16_t j=0;j<numUePerEnb;j++){
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        positionAlloc->Add (Vector (distance*(i%3+1), distance*(i/3+1), 0));
        mobility.SetPositionAllocator(positionAlloc);
        mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (distance*(i%3+1)-20, distance*(i%3+1)+20, distance*(i/3+1)-20, distance*(i/3+1)+20)));
        mobility.Install(ueNodes.Get(i*numUePerEnb+j));
      }
    }

    // Install LTE Devices to the nodes
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

    // Install the IP stack on the UEs
    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
    for (uint16_t u = 0; u < ueNodes.GetN (); ++u)
      {
        Ptr<Node> ueNode = ueNodes.Get (u);
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
    // std::vector<ClientWithMessages> clients;
    // RngSeedManager::SetSeed (3);
    // Ptr<UniformRandomVariable> randomNumGenerator = CreateObject<UniformRandomVariable> ();

    // for(uint16_t i=0;i<numClient;i++){
    //   uint16_t randomNum = randomNumGenerator->GetInteger ();
    //   ClientWithMessages vStompClient;
    //   vStompClient.node = ueNodes.Get(randomNum);
    //   std::vector<std::string> channels;
    //   channels.push_back("hhh");
    //   channels.push_back("xixi");
    //   std::vector<Message> messages;
    //   Message message1;
    //   message1.target="zzj";
    //   message1.content="I'm client "+std::to_string(i)+" from UE "+std::to_string(randomNum);
    //   Message message2;
    //   message2.target="hhh";
    //   message2.content="try out my channel";
    //   messages.push_back(message1);
    //   messages.push_back(message2);
    //   vStompClient.userName="zzj";
    //   vStompClient.channelsTosub = channels;
    //   vStompClient.messagesToSend = messages;
    //   // ClientWithMessages client2;
    //   // ClientWithMessages client3;
    //   clients.push_back(vStompClient);
    // }


    //client1
    ClientWithMessages client1;
    // client1.node = pgw;
    client1.node = ueNodes.Get(0);
    std::vector<std::string> channels;
    channels.push_back("LifeInTongji");
    channels.push_back("ComputerNetWork");
    std::vector<Message> messages;
    Message message1;
    message1.target="zzj";
    message1.content="say hello to myself";
    Message message2;
    message2.target="ComputerNetWork";
    message2.content="try out my channel";
    messages.push_back(message1);
    messages.push_back(message2);
    client1.userName="zzj";
    client1.channelsTosub = channels;
    client1.messagesToSend = messages;

    //client2
    ClientWithMessages client2;
    client2.userName = "lh";
    std::vector<std::string> channel2;
    channel2.push_back("ComputerNetWork");
    std::vector<Message> messages2;
    Message message2_1;
    message2_1.target = "zzj";
    message2_1.content = "Hello! My name is lh, nice to meet you!";
    Message message2_2;
    message2_2.target="ComputerNetWork";
    message2_2.content="I have a question! Why is TCP/IP important?";
    messages2.push_back(message2_1);
    messages2.push_back(message2_2);
    client2.channelsTosub = channel2;
    client2.messagesToSend = messages2;
    // 主要是这里应该选择哪个点？
    client2.node = ueNodes.Get(1);

    //client 3
    ClientWithMessages client3;
    client3.userName = "jxh";
    std::vector<std::string> channel3;
    channel3.push_back("LifeInTongji");
    std::vector<Message> messages3;
    Message message3_1;
    message3_1.target = "lh";
    message3_1.content = "You are my roommate!";
    Message message3_2;
    message3_2.target="ComputerNetWork";
    message3_2.content="I don't need to subscribe to this channel to send you a message!";
    messages3.push_back(message3_1);
    messages3.push_back(message3_2);
    client3.channelsTosub = channel3;
    client3.messagesToSend = messages3;
    // 主要是这里应该选择哪个点？
    client3.node = ueNodes.Get(2);

    //client 4
    ClientWithMessages client4;
    client4.userName = "lym";
    std::vector<std::string> channel4;
    channel4.push_back("hhh");
    std::vector<Message> messages4;
    Message message4_1;
    message4_1.target = "jxh";
    message4_1.content = "Have you take your breakfast?";
    Message message4_2;
    message4_2.target="ComputerNetWork";
    message4_2.content="Have you finished your homework?";
    messages4.push_back(message4_1);
    messages4.push_back(message4_2);
    client4.channelsTosub = channel4;
    client4.messagesToSend = messages4;
    // 主要是这里应该选择哪个点？
    client4.node = ueNodes.Get(3);

    // ClientWithMessages client2;
    // ClientWithMessages client3;
    std::vector<ClientWithMessages> clients;
    clients.push_back(client1);
    clients.push_back(client2);
    clients.push_back(client3);
    clients.push_back(client4);






    vStompApplication application = vStompApplication(remoteHost,remoteHostAddr,clients);
    application.start();

    Simulator::Stop(Seconds(50));
    NS_LOG_INFO("The simulation begins");

    if (tracing == true)
    {
      p2ph.EnablePcapAll ("F4_ns3");
//      lteHelper->EnableTraces();
    }

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}

