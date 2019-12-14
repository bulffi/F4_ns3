NS_LOG_COMPONENT_DEFINE("F4ComputerNetwork");

enum Command{
    CONNECT,
    CONNECTED,
    SUBSCRIBE,
    SEND,
    DISCONNECT,
    UNSUBSCRIBE,
};
class Head{
    public:
        std::string getSubscribe(){
            for(auto iter = info.begin(); iter != info.end(); iter ++){
                if(iter->first == "channel"){
                    return iter->second;
                }
            }
            return "";
        }
        std::string getTarget(){
            for(auto iter = info.begin(); iter != info.end(); iter ++){
                if(iter->first == "target"){
                    return iter->second;
                }
            }
            return "";
        }
        std::string getUserName(){
            for(auto iter = info.begin(); iter != info.end(); iter ++){
                if(iter->first == "name"){
                    return iter->second;
                }
            }
            return "";
        }
        std::map<std::string, std::string> info;
};

class Body{
    public:
        std::string content;
};
class Frame{
    public:
        Command command;
        Head head;
        Body body;
};

std::string FrameToString(Frame frame){
    std::string answer = "";
    switch (frame.command)
    {
    case CONNECT:{
        answer += "CONNECT";
    }break;
    case CONNECTED:{
        answer += "CONNETED";
    }break;
    case SUBSCRIBE:{
        answer += "SUBSCRIBE";
    }break;
    case SEND:{
        answer += "SEND";
    }break;
    case DISCONNECT:{
        answer += "DISCONNECT";
    }break;
    case UNSUBSCRIBE:{
        answer += "UNSUBSCRIBE";
    }
    default:
        break;
    }
    answer += "\n";
    for(auto iter = frame.head.info.begin(); iter != frame.head.info.end(); iter ++){
        std::string key = iter->first;
        std::string value = iter->second;
        answer += key + ":" + value + "\n";
    }
    answer += "\n";
    answer += frame.body.content;
    answer += "\000";
    return answer;
}

Frame StringToFrame(std::string stompString){
    Frame frame;
    std::stringstream stream(stompString);
    std::string frameCommand;
    std::getline(stream,frameCommand);
    Command command;
    if (frameCommand == "CONNECT"){
        command = CONNECT;
    }else if(frameCommand == "CONNETED"){
        command = CONNECTED;
    }else if(frameCommand == "SUBSCRIBE"){
        command = SUBSCRIBE;
    }else if(frameCommand == "SEND"){
        command = SEND;
    }else if(frameCommand == "DISCONNECT"){
        command = DISCONNECT;
    }else if(frameCommand == "UNSUBSCRIBE")
    {
        command = UNSUBSCRIBE;
    }
    
    std::string line;
    Head head;
    while (std::getline(stream, line) && line!="")
    {
        int index = line.find_first_of(":");
        std::string key = line.substr(0,index);
        std::string value = line.substr(index + 1, line.length());
        head.info.insert(std::make_pair(key,value));
    }
    Body body;
    while (std::getline(stream, line))
    {
        body.content += line;
    }
    frame.body = body;
    frame.command = command;
    frame.head = head;
    return frame;
}