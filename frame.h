enum Command{
    CONNECT,
    SUBSCRIBE,
    SEND,
    DISCONNECT,
};
class Head{
    public:
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

std::shared_ptr<std::string> FrameToString(std::shared_ptr<Frame> frame){

    return std::shared_ptr<std::string>(new std::string(" hello "));
}

std::shared_ptr<Frame> StringToFrame(std::string stompString){

    return std::shared_ptr<Frame>( new Frame());
}