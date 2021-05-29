#include "net.hpp"
//#include "net.cpp"

void sendThread(){
    while (true){

    }
}

int main(){
    TinyNet *n = new TinyNet(30);
    TinyUdpPortNumber portNum = TinyUdpPortNumber{0,0};
    TinyConnection* con = n->InitConnection(portNum,29);
    char data[TINYUDP_PAYLOAD_MAX] = "aiueo";
    con->Send(n->GetRoute(),data,TINYUDP_PAYLOAD_MAX);
}
