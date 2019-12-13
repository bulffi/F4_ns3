#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "vstomp.h"
using namespace ns3;

int main(int argc, char* argv[]){

    CommandLine cmd;
    cmd.Parse (argc, argv);
    
    Time::SetResolution (Time::NS);
    LogComponentEnable("F4ComputerNetwork",LOG_LEVEL_INFO);
    // Physical & link
    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);
    

    // inter-networking
    InternetStackHelper stack;
    stack.Install (nodes);

    NS_LOG_INFO("hello world");
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    // transport





    // 应用层
    // 需要的输入是一个已经安装了 TCP 协议的 server 以及它的 ns3::Ipv4Address 
    // 一个数组，包含一组已经安装了 TCP 协议的 client
    ClientWithMessages client1;
    client1.node = nodes.Get(1);
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
    vStompApplication application = vStompApplication(nodes.Get(0),interfaces.GetAddress(0),clients);
    application.start();

    Simulator::Stop(Seconds(20));
    NS_LOG_INFO("The simulation begins");
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}

