#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include"frame.h"
using namespace ns3;
NS_LOG_COMPONENT_DEFINE("F4ComputerNetwork");

void setServerSocket(Ptr<Socket> serverSocket, uint32_t localPort);
void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port);
void connectionSucceeded(Ptr<Socket> socket);
void receiveData(Ptr<Socket> socket);
void handleAccept(Ptr<Socket> socket, const Address& from);
void getServerSetUp(Ptr<Socket> serverSocket);

class vStompServer;
class Guard;
class vStompClient;
class Transmitter;






class vStompApplication {
    public :
        vStompApplication(Ptr<Node> serverNode, Ipv4Address serverAddress, std::vector<Ptr<Node>> clients){
        serverSocket = Socket::CreateSocket(serverNode,TcpSocketFactory::GetTypeId());
        clientSocket = Socket::CreateSocket(clients.at(0),TcpSocketFactory::GetTypeId());
        Simulator::Schedule(Seconds(0.9),&getServerSetUp, serverSocket);
        Simulator::Schedule(
            Seconds(1.0),
            &startFlow, clientSocket, serverAddress, 8080
        );
        // getServerSetUp();
        }
        void start(){}
    private:
        Ptr<Socket> serverSocket;
        Ptr<Socket> clientSocket;
        std::map<int,std::shared_ptr<vStompServer>> serverPool;
        // void getServerSetUp();
    
};


class vStompServer{
    private:
        std::shared_ptr<std::map<std::string,std::set<std::shared_ptr<vStompClient>>>> clientPool;
        std::shared_ptr<Guard> guard;
    public:
        void addClient(std::shared_ptr<vStompClient> client){

        }
        void deleteClient(std::shared_ptr<std::string> clientName){

        }
        void sendFrameTo(std::shared_ptr<std::string>, Ptr<Frame> frame){

        }

        vStompServer(Ptr<Socket> serverSocket);
        
};

class Guard{
    private:
        std::string CLASSNAME = "Guard:   ";
        std::shared_ptr<vStompServer> server;
        Ptr<Socket> socket;
    public:
        Guard(Ptr<Socket> psocket, std::shared_ptr<vStompServer> pserver, uint32_t port){
            server = pserver;
            socket = psocket;
            setServerSocket(socket, port);
        }

        void setServerSocket(Ptr<Socket> serverSocket, uint32_t localPort){
            serverSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), localPort));
            serverSocket->Listen();
            serverSocket->SetRecvCallback(MakeCallback(&Guard::receiveData, this));
            serverSocket->SetAcceptCallback(
                MakeNullCallback<bool,Ptr<Socket>,const Address&>(),
                MakeCallback(&Guard::handleAccept, this)
            );
         }
         void handleAccept(Ptr<Socket> socket, const Address& from){
            NS_LOG_INFO( CLASSNAME + "A socket connection is established."); 
            socket->SetRecvCallback(MakeCallback(&Guard::receiveData, this));
        }
        void receiveData(Ptr<Socket> socket){
            NS_LOG_INFO(CLASSNAME + "Get a packet.");
            Ptr<Packet> pkt = socket->Recv();
            std::ostringstream* contentStream = new std::ostringstream();
            pkt->CopyData(contentStream,(uint32_t)INT_MAX);
            std::string content = (*contentStream).str();
            std::shared_ptr<Frame> frame = StringToFrame(content);
            dealWithFrame(frame);
        }
        void dealWithFrame(std::shared_ptr<Frame> frame);
};


class vStompClient{
    private:
        std::shared_ptr<Transmitter> server;
        std::shared_ptr<std::string> name;
    public:
        void getFrame(std::shared_ptr<Frame> frame){
        }
        void sendFrame(std::shared_ptr<Frame> frame){
        }
};

class Transmitter{
    private:
        Ptr<Socket> targetSocket;
    public:
        void sendFrame(std::shared_ptr<Frame> frame){

        }
};








void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port){
    clientSocket->Bind();
    clientSocket->Connect(InetSocketAddress(address,port));
    clientSocket->SetConnectCallback(MakeCallback(&connectionSucceeded),MakeNullCallback<void,Ptr<Socket>>());
}

void connectionSucceeded(Ptr<Socket> socket){
    socket->Send(Create<Packet>(512));
    Simulator::Schedule(Seconds(4),connectionSucceeded,socket);
}












void getServerSetUp(Ptr<Socket> serverSocket){
    // std::shared_ptr<vStompServer> server = std::shared_ptr<vStompServer>( new vStompServer(serverSocket));
    new vStompServer(serverSocket);
    //int size = serverPool.size();
    //serverPool.insert(std::make_pair(size,server));
}

vStompServer::vStompServer(Ptr<Socket> serverSocket){
    guard = std::shared_ptr<Guard>(new Guard(serverSocket,std::shared_ptr<vStompServer>(this),8080));
}

void Guard::dealWithFrame(std::shared_ptr<Frame> frame){
    NS_LOG_INFO("Parse the frame");
    switch ((frame->command))
    {
        case CONNECT:{
            NS_LOG_INFO("This is a connect message");
        }break;
        case SUBSCRIBE:{

        }break;
        case SEND:{

        }break;
        case DISCONNECT:{

        }break;
        default:
            break;
    }
}