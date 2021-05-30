#include "net.hpp"
//#include "net.cpp"

void sendThread(){
    while (true){
        break;
    }
}
void recvThread(TinyNet* n, RoutingTable *routes){
    while (true){
        TinyRipPacket *ripp = new TinyRipPacket();
        ripp->flag = TinyRipFlag();
        ripp->hops = Hops{5,4,3,2};
        TinyIpPacket *p = new TinyIpPacket();
        TinyIp::SetPayload(p, (char*)ripp, 99);
        p->src = 5;
        p->dst = 30;
        p->flag = IpFlag{0,PROTOCOL_TINY_RIP,0,0,0};
        n->movePacketToQueue(*p);
        break;
    }
}

void handleThread(TinyNet* n, RoutingTable *routes){
    while(true){
        n->HandleAllPackets(routes);
        break;
    }
}


int main(){
    TinyNet *n = new TinyNet(30);
    n->GetRoute()->dumpRoutingTable();
    TinyUdpPortNumber portNum = TinyUdpPortNumber{0,0};
    TinyConnection* con = n->InitConnection(portNum,29);
    char data[TINYUDP_PAYLOAD_MAX] = "aiueo";
    con->Send(n->GetRoute(),data,TINYUDP_PAYLOAD_MAX);
    recvThread(n, n->GetRoute());
    handleThread(n, n->GetRoute());
    n->GetRoute()->dumpRoutingTable();
}
