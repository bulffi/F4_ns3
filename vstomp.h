#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("F4ComputerNetwork");

void setServerSocket(Ptr<Socket> serverSocket, uint32_t localPort);
void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port);
void connectionSucceeded(Ptr<Socket> socket);
void receiveData(Ptr<Socket> socket);
void handleAccept(Ptr<Socket> socket, const Address& from);

class vStompApplication {
    public :
    vStompApplication(Ptr<Node> serverNode, Ipv4Address serverAddress, std::vector<Ptr<Node>>* clients){
    Ptr<Socket> serverSocket = Socket::CreateSocket(serverNode,TcpSocketFactory::GetTypeId());
    Ptr<Socket> clientSocket = Socket::CreateSocket(clients->at(0),TcpSocketFactory::GetTypeId());

    Simulator::ScheduleNow( &setServerSocket, serverSocket, 8080);
    Simulator::Schedule(
        Seconds(1.0),
        &startFlow, clientSocket, serverAddress, 8080
    );
    }
    void start(){}
private:
    
};

void setServerSocket(Ptr<Socket> serverSocket, uint32_t localPort){
    serverSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), localPort));
    serverSocket->Listen();
    serverSocket->SetRecvCallback(MakeCallback(&receiveData));
    serverSocket->SetAcceptCallback(
        MakeNullCallback<bool,Ptr<Socket>,const Address&>(),
        MakeCallback(&handleAccept)
    );
}

void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port){
    clientSocket->Bind();
    clientSocket->Connect(InetSocketAddress(address,port));
    clientSocket->SetConnectCallback(MakeCallback(&connectionSucceeded),MakeNullCallback<void,Ptr<Socket>>());
}

void connectionSucceeded(Ptr<Socket> socket){
    socket->Send(Create<Packet>(512));
}
void receiveData(Ptr<Socket> socket){
    NS_LOG_INFO("+ "<<"get a data");
    Ptr<Packet> pkt = socket->Recv();
    NS_LOG_INFO("The size of the data is " +  std::to_string(pkt->GetSize()));

}
void handleAccept(Ptr<Socket> socket, const Address& from){
    socket->SetRecvCallback(MakeCallback(&receiveData));
}