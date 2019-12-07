#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include"frame.h"
using namespace ns3;

void setServerSocket(Ptr<Socket> serverSocket, uint32_t localPort);
void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port);
void connectionSucceeded(Ptr<Socket> socket);
void receiveData(Ptr<Socket> socket);
void handleAccept(Ptr<Socket> socket, const Address& from);
void getServerSetUp(Ptr<Socket> serverSocket);
void display(Ptr<Socket> socket);
void registerAnother(Ptr<Socket> socket);
void registerAnother(Ptr<Socket> socket);
void sendFromZZJTOSkr(Ptr<Socket> socket);

class vStompServer;
class Guard;
class vStompClient;
class Transmitter;

vStompServer* serverPtr;

class Transmitter{
    private:
        Ptr<Socket> targetSocket;
    public:
        void sendFrame(Frame frame){
            std::string frameString = FrameToString(frame);
            targetSocket->Send(Create<Packet>(
                reinterpret_cast<const uint8_t*>(frameString.c_str()),frameString.length())
            );
        }
        Transmitter(Ptr<Socket> ptargetSocket){
            targetSocket = ptargetSocket;
        }
        Transmitter(){}
};

class vStompClient{
    private:
        Transmitter socketToClient;
        std::string name;
    public:
        void getFrame(Frame frame){
            socketToClient.sendFrame(frame);
        }
        vStompClient(Transmitter trans, std::string aName){
            socketToClient = trans;
            name = aName;
        }
        std::string getName(){
            return name;
        }
};


class vStompApplication {
    public :
        vStompApplication(Ptr<Node> serverNode, Ipv4Address serverAddress, std::vector<Ptr<Node>> clients){
        serverSocket = Socket::CreateSocket(serverNode,TcpSocketFactory::GetTypeId());
        clientSocket = Socket::CreateSocket(clients.at(0),TcpSocketFactory::GetTypeId());
        Simulator::Schedule(Seconds(0.1),&getServerSetUp, serverSocket);
        Simulator::Schedule(
            Seconds(1.0),
            &startFlow, clientSocket, serverAddress, 8080
        );
        }
        void start(){}
    private:
        Ptr<Socket> serverSocket;
        Ptr<Socket> clientSocket;
        std::map<int,std::shared_ptr<vStompServer>> serverPool;
    
};


class vStompServer{
    private:
        std::string CLASSNAME = "Server:    ";
        std::map<std::string,std::set<std::shared_ptr<vStompClient>>> clientPool;
        std::shared_ptr<Guard> guard;
    public:
        void addClient(std::shared_ptr<vStompClient> client){
            std::string name = client->getName();
            std::set<std::shared_ptr<vStompClient>> subscribers;
            subscribers.insert(client);
            clientPool[name] = subscribers;
            NS_LOG_INFO(CLASSNAME +  name + " is registered");
        }
        void deleteClient(std::string clientName){
            auto iter = clientPool.find(clientName);
            if(iter!=clientPool.end()){
                clientPool.erase(iter);
            }
            NS_LOG_INFO(CLASSNAME + clientName + " is leaving.");
        }
        void sendFrameTo(std::string channelName, Frame frame){
            auto iter = clientPool.find(channelName);
            if(iter!=clientPool.end()){
                std::set<std::shared_ptr<vStompClient>> subscribers = iter->second;
                for(auto iter = subscribers.begin();iter!=subscribers.end();iter++){
                    (*iter)->getFrame(frame);
                }
            }
            NS_LOG_INFO(CLASSNAME + "Send frame to every subs of " + channelName);
        }
        void addSubscription(std::string userName, std::string channelName){
            std::set<std::shared_ptr<vStompClient>> temptClient = clientPool[userName];
            std::shared_ptr<vStompClient> client = *(temptClient.begin());
            std::set<std::shared_ptr<vStompClient>> channelSubsribers = clientPool[channelName];
            channelSubsribers.insert(client);
            clientPool[channelName] = channelSubsribers;
            NS_LOG_INFO(CLASSNAME + userName +" is add to group " + channelName);
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
            Frame frame = StringToFrame(content);
            dealWithFrame(frame,socket);
        }
        void dealWithFrame(Frame frame,Ptr<Socket> socket);
};

void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port){
    clientSocket->Bind();
    clientSocket->Connect(InetSocketAddress(address,port));
    clientSocket->SetConnectCallback(MakeCallback(&connectionSucceeded),MakeNullCallback<void,Ptr<Socket>>());
    clientSocket->SetRecvCallback(MakeCallback(&display));
}

void display(Ptr<Socket> socket){
    Ptr<Packet> packet = socket->Recv();
    std::ostringstream* contentStream = new std::ostringstream();
    packet->CopyData(contentStream,(uint32_t)INT_MAX);
    std::string content = (*contentStream).str();
    NS_LOG_INFO("Client    :\" " + content  + " \"");
}

void connectionSucceeded(Ptr<Socket> socket){
    std::string content = "CONNECT\nname:zzj\n\n\000";
    socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(content.c_str()),content.length()));
    
    
    Simulator::Schedule(Seconds(4),registerAnother,socket);
}



void sendInfoToChannel(Ptr<Socket> socket){
    std::string greeting = "SEND\ntarget:hhh\n\nTry out the channel!\000";
    socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(greeting.c_str()), greeting.length()));
}

void subsribeToChannel(Ptr<Socket>socket){
    std::string subs = "SUBSCRIBE\nchannel:hhh\nname:zzj\n\n\000";
     socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(subs.c_str()),subs.length()));
    Simulator::Schedule(Seconds(1),sendInfoToChannel,socket);
}

void registerAnother(Ptr<Socket> socket){
    std::string content = "CONNECT\nname:skr\n\n\000";
    socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(content.c_str()), content.length()));
    
    Simulator::Schedule(Seconds(4),sendFromZZJTOSkr,socket);
}

void sendFromZZJTOSkr(Ptr<Socket> socket){
    std::string content = "SEND\ntarget:skr\n\nHello,but what is that \000";
    socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(content.c_str()), content.length()));
    Simulator::Schedule(Seconds(1),subsribeToChannel,socket);
}

void getServerSetUp(Ptr<Socket> serverSocket){
    serverPtr = new vStompServer(serverSocket);
}

vStompServer::vStompServer(Ptr<Socket> serverSocket){
    guard = std::shared_ptr<Guard>(new Guard(serverSocket,std::shared_ptr<vStompServer>(this),8080));
}

void Guard::dealWithFrame(Frame frame,Ptr<Socket> socket){
    NS_LOG_INFO(CLASSNAME + "Parse the frame");
    switch ((frame.command))
    {
        case CONNECT:{
            NS_LOG_INFO(CLASSNAME + "This is a connect message");
            Transmitter transmiter(socket);
            std::shared_ptr<vStompClient> client(new vStompClient(transmiter,frame.head.getUserName()));
            server->addClient(client);
            Frame frame;
            frame.command = CONNECTED;
            client->getFrame(frame);
        }break;
        case SUBSCRIBE:{
            NS_LOG_INFO(CLASSNAME + "This is a subscribe message");
            server->addSubscription(frame.head.getUserName(),frame.head.getSubscribe());
        }break;
        case SEND:{
            NS_LOG_INFO(CLASSNAME + "This is a send message");
            server->sendFrameTo(frame.head.getTarget(), frame);
        }break;
        case DISCONNECT:{
            NS_LOG_INFO(CLASSNAME + "This is a disconnect message");
            server->deleteClient(frame.head.getUserName());
        }break;
        default:
            break;
    }
}