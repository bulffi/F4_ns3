#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <memory>
#include"frame.h"
using namespace ns3;

class Message;

void setServerSocket(Ptr<Socket> serverSocket, uint32_t localPort);
void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port, std::string userName,std::vector<std::string> _channelsToSubsribe, std::vector<Message> _messagesToSend);
void receiveData(Ptr<Socket> socket);
void handleAccept(Ptr<Socket> socket, const Address& from);
void getServerSetUp(Ptr<Socket> serverSocket);
void display(Ptr<Socket> socket);
void registerAnother(Ptr<Socket> socket);
void registerAnother(Ptr<Socket> socket);
void sendFromZZJTOSkr(Ptr<Socket> socket);
void sendInfoToChannel(Ptr<Socket> socket, std::string target, std::string content);
void subsribeToChannel(Ptr<Socket>socket,std::string channelName,std::string userName);
void disConnect(Ptr<Socket>socket, std::string clientName);
void unsubscribeToChannel(Ptr<Socket>socket, std::string userName, std::string channelToUnsubscribe);
class vStompServer;
class ConnectionPool;
class ClientStub;
class Transmitter;

vStompServer* serverPtr;

// transmitter is a simple encap of socket
// it keeps a socket inside and provide an interface for sending message using it
// void sendFrame(Frame frame)
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

// client kept in the server. not real client, just easy to manage for server
class ClientStub{
    private:
        Transmitter socketToClient;
        std::string name;
    public:
        void getFrame(Frame frame){
            socketToClient.sendFrame(frame);
        }
        ClientStub(Transmitter trans, std::string aName){
            socketToClient = trans;
            name = aName;
        }
        std::string getName(){
            return name;
        }
};

class Message{
    public:
        std::string target;
        std::string content;
};

class ClientWithMessages{
    public:
        Ptr<Node> node;
        // Ptr<Socket> clientSocket;
        std::string userName;
        std::vector<std::string> channelsTosub;
        std::vector<Message> messagesToSend;
};

class vStompApplication {
    public :
        vStompApplication(Ptr<Node> serverNode, Ipv4Address serverAddress, std::vector<ClientWithMessages> clients){
            serverSocket = Socket::CreateSocket(serverNode,TcpSocketFactory::GetTypeId());
            Simulator::Schedule(Seconds(0.1),&getServerSetUp, serverSocket);
            for(uint16_t i=0;i<clients.size();i++) {
                Ptr<Socket> clientSocket = Socket::CreateSocket(clients[i].node, TcpSocketFactory::GetTypeId());
                Simulator::Schedule(
                    Seconds(2.0 + i*0.02),
                    &startFlow, clientSocket, serverAddress, 8080, clients[i].userName, clients[i].channelsTosub,clients[i].messagesToSend
                );
            }
        }
        void start(){}
    private:
        Ptr<Socket> serverSocket;
        Ptr<Socket> clientSocket;
        std::map<int,std::shared_ptr<vStompServer>> serverPool;
    
};



// internal state is kept here
class vStompServer{
    private:
        std::string CLASSNAME = "Server";
        std::map<std::string,std::set<std::shared_ptr<ClientStub>>> clientPool;
        std::shared_ptr<ConnectionPool> connectionPool;
        int id;
        std::map<int,vStompServer*> fellows;
        std::string serverAddress;
        std::string port = "8080";
    public:
        std::string getIP(){return serverAddress;}
        void setFellows(std::map<int,vStompServer*> fellowMap){
            fellows = fellowMap;
        }
        void setID(int _id){id = _id;}
        void addClient(std::shared_ptr<ClientStub> client){
            std::string name = client->getName();
            std::set<std::shared_ptr<ClientStub>> subscribers;
            subscribers.insert(client);
            clientPool[name] = subscribers;
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " +  name + " is registered");
        }
        void deleteClient(std::string clientName){
            for(auto iter = clientPool.begin();iter!=clientPool.end();){
                if(iter->first == clientName){
                    clientPool.erase(iter ++ );
                }else{
                    std::set<std::shared_ptr<ClientStub>> subscribers = iter->second;
                    for(auto subsIter = subscribers.begin();subsIter!=subscribers.end();){
                        if((*subsIter)->getName()==clientName){
                            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " + clientName + " unsubscribe " + iter->first);
                            subscribers.erase(subsIter++);
                            iter->second = subscribers;
                            break;
                        }else{
                            subsIter ++;
                        }
                    }
                    iter ++;
                }

            }
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " + clientName + " is leaving.");
        }
        void unsubscribeFromChannel(std::string userName,std::string channelToAbandon){
            bool finished = false;
            for(auto iter = clientPool.begin();iter!=clientPool.end();iter++){
                if(iter->first == channelToAbandon){
                    std::set<std::shared_ptr<ClientStub>> subscribers = iter->second;
                    for(auto subsIter = subscribers.begin();subsIter!=subscribers.end();){
                        if((*subsIter)->getName()==userName){
                            subscribers.erase(subsIter++);
                            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " + userName + " unsubscribe " + channelToAbandon);
                            iter->second = subscribers;
                            finished = true;
                            break;
                        }else{
                            subsIter ++;
                        }
                    }
                }
                if(finished){
                    break;
                }
            }
        }
        void sendFrameTo(std::string channelName, Frame frame){
            auto iter = clientPool.find(channelName);
            if(iter!=clientPool.end()){
                std::set<std::shared_ptr<ClientStub>> subscribers = iter->second;
                for(auto iter = subscribers.begin();iter!=subscribers.end();iter++){
                    (*iter)->getFrame(frame);
                }
            }
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " + "Send frame to every subs of " + channelName);
        }
        void addSubscription(std::string userName, std::string channelName){
            // the client automatically subscribe to a channel with the name of himself
            // we trace the user by this
            std::set<std::shared_ptr<ClientStub>> temptClient = clientPool[userName];
            std::shared_ptr<ClientStub> client = *(temptClient.begin());
            std::set<std::shared_ptr<ClientStub>> channelSubsribers = clientPool[channelName];
            channelSubsribers.insert(client);
            clientPool[channelName] = channelSubsribers;
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " + userName +" is add to group " + channelName);
        }
        vStompServer(Ptr<Socket> serverSocket);
};

class ConnectionPool{
    private:
        std::string CLASSNAME = "ConnectionPool";
        std::shared_ptr<vStompServer> server;
        Ptr<Socket> socket;
    public:
        ConnectionPool(Ptr<Socket> psocket, std::shared_ptr<vStompServer> pserver, uint32_t port){
            server = pserver;
            socket = psocket;
            setServerSocket(socket, port);
        }

        void setServerSocket(Ptr<Socket> serverSocket, uint32_t localPort){
            serverSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), localPort));
            serverSocket->Listen();
            serverSocket->SetRecvCallback(MakeCallback(&ConnectionPool::receiveData, this));
            serverSocket->SetAcceptCallback(
                MakeNullCallback<bool,Ptr<Socket>,const Address&>(),
                MakeCallback(&ConnectionPool::handleAccept, this)
            );
         }
         void handleAccept(Ptr<Socket> socket, const Address& from){
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "A socket connection is established."); 
            socket->SetRecvCallback(MakeCallback(&ConnectionPool::receiveData, this));
        }
        void receiveData(Ptr<Socket> socket){
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "Get a packet.");
            Ptr<Packet> pkt = socket->Recv();
            std::ostringstream* contentStream = new std::ostringstream();
            pkt->CopyData(contentStream,(uint32_t)INT_MAX);
            std::string content = (*contentStream).str();
            Frame frame = StringToFrame(content);
            dealWithFrame(frame,socket);
        }
        void dealWithFrame(Frame frame,Ptr<Socket> socket){
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "Parse the frame");
            switch ((frame.command))
            {
                case CONNECT:{
                    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "This is a connect message");
                    Transmitter transmiter(socket);
                    std::shared_ptr<ClientStub> client(new ClientStub(transmiter,frame.head.getUserName()));
                    server->addClient(client);
                    Frame frame;
                    frame.command = CONNECTED;
                    client->getFrame(frame);
                }break;
                case SUBSCRIBE:{
                    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "This is a subscribe message");
                    server->addSubscription(frame.head.getUserName(),frame.head.getSubscribe());
                }break;
                case SEND:{
                    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "This is a send message");
                    server->sendFrameTo(frame.head.getTarget(), frame);
                }break;
                case DISCONNECT:{
                    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "This is a disconnect message");
                    server->deleteClient(frame.head.getUserName());
                }break;
                case UNSUBSCRIBE:{
                    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:     " + "This is a unsubscribe message");
                    server->unsubscribeFromChannel(frame.head.getUserName(), frame.head.getSubscribe());
                }break;
                default:
                    break;
            }
        }
};

class Client{
    private:
        std::vector<std::string> channelsToSebscribe;
        std::vector<Message> messagesToSend;
        Ptr<Socket> clientSocket;
        std::string userName;
        std::string CLASSNAME = "Client";
        void connectionSucceeded(Ptr<Socket> socket){
            std::string content = "CONNECT\nname:" + userName + "\n\n\000";
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " + "SENDING the following frame\n" 
            + "==========================================\n" + content +"\n==========================================");
            socket->Send(Create<Packet>(
            reinterpret_cast<const uint8_t*>(content.c_str()),content.length()));
            for(uint16_t i =0;i<channelsToSebscribe.size();i++){
                Simulator::Schedule(Seconds(4 + i*0.1), &subsribeToChannel, socket, channelsToSebscribe[i],userName);
            }
            for(uint16_t i=0;i<messagesToSend.size();i++){
                Simulator::Schedule(Seconds(8 + channelsToSebscribe.size()*0.1 + 1 + i*0.1), &sendInfoToChannel, socket, messagesToSend[i].target , messagesToSend[i].content);
            }
            double totalTimeConsume = channelsToSebscribe.size()*0.1 + messagesToSend.size()*0.1 + 1;
            // Simulator::Schedule(Seconds(4 + totalTimeConsume + 0.5), &unsubscribeToChannel,socket,userName,"hhh");
            Simulator::Schedule(Seconds(15 + totalTimeConsume + 1),&disConnect,socket,userName);
        } 
        void display(Ptr<Socket> socket){
            Ptr<Packet> packet = socket->Recv();
            std::ostringstream* contentStream = new std::ostringstream();
            packet->CopyData(contentStream,(uint32_t)INT_MAX);
            std::string content = (*contentStream).str();
            NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s ["+ CLASSNAME +"]:             " + userName + " GETTING the following frame\n"  
            + "==========================================\n" + content  + "\n==========================================");
        }
    public:
        Client(Ptr<Socket> _clientSocket, Ipv4Address address, uint32_t port, std::string _userName,std::vector<std::string> _channelsToSubsribe, std::vector<Message> _messagesToSend){
            clientSocket = _clientSocket;
            channelsToSebscribe = _channelsToSubsribe;
            messagesToSend = _messagesToSend;
            userName = _userName;
            clientSocket->Bind();
            clientSocket->Connect(InetSocketAddress(address,port));
            clientSocket->SetConnectCallback(MakeCallback(&Client::connectionSucceeded,this),MakeNullCallback<void,Ptr<Socket>>());
            clientSocket->SetRecvCallback(MakeCallback(&Client::display,this));
        }
};

void startFlow(Ptr<Socket> clientSocket, Ipv4Address address, uint32_t port, std::string userName,std::vector<std::string> _channelsToSubsribe, std::vector<Message> _messagesToSend){
    new Client(clientSocket,address,port,userName,_channelsToSubsribe,_messagesToSend);
}

void sendInfoToChannel(Ptr<Socket> socket, std::string target, std::string content){
    std::string greeting = "SEND\ntarget:" +target +"\n\n" + content +"\000";
     NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s [Client]:             " + "SENDING the following frame\n" 
            + "==========================================\n" + greeting +"\n==========================================");
    socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(greeting.c_str()), greeting.length()));
}

void subsribeToChannel(Ptr<Socket>socket,std::string channelName,std::string userName){
    std::string subs = "SUBSCRIBE\nchannel:" + channelName + "\nname:" + userName + "\n\n\000";
    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s [Client]:             " + "SENDING the following frame\n" 
            + "==========================================\n" + subs +"\n==========================================");
     socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(subs.c_str()),subs.length()));
}

void disConnect(Ptr<Socket>socket, std::string clientName){
    std::string disconnect = "DISCONNECT\nname:" + clientName +"\n\n\000";
    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s [Client]:             " + "SENDING the following frame\n" 
            + "==========================================\n" + disconnect +"\n==========================================");
    socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(disconnect.c_str()),disconnect.length()));
}

void unsubscribeToChannel(Ptr<Socket>socket, std::string userName, std::string channelToUnsubscribe){
    std::string unsubscribe = "UNSUBSCRIBE\nname:" + userName + "\nchannel:" + channelToUnsubscribe +"\n\n\000";
    NS_LOG_INFO(std::to_string(Simulator::Now().GetSeconds()) +"s [Client]:             " + "SENDING the following frame\n" 
            + "==========================================\n" + unsubscribe +"\n==========================================");
    socket->Send(Create<Packet>(
        reinterpret_cast<const uint8_t*>(unsubscribe.c_str()),unsubscribe.length()));
}

void getServerSetUp(Ptr<Socket> serverSocket){
    serverPtr = new vStompServer(serverSocket);
}

vStompServer::vStompServer(Ptr<Socket> serverSocket){
    connectionPool = std::shared_ptr<ConnectionPool>(new ConnectionPool(serverSocket,std::shared_ptr<vStompServer>(this),8080));
}
