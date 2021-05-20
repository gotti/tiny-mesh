#include <string.h>
#include "net.hpp"

void TinyIp::SetFlag(TinyIpPacket *p, IpFlag f){
    p->flag = f; //?
}
void TinyIp::SetDst(TinyIpPacket *p, Address dst){
    p->dst = dst;
}
void TinyIp::SetSrc(TinyIpPacket *p, Address src){
    p->src = src;
}
void TinyIp::SetHop(TinyIpPacket *p, Address *hop){
    memcpy(p->hops, hop, sizeof(Address)*4); //?
}
void TinyIp::SetPayload(TinyIpPacket *p, char* payload, int length){
    memset(p->payload, 0, 24);
    memcpy(p->payload, payload, length>24 ? 24 : length);
}


void TinyUdp::SetFlag(TinyUdpPacket *p, TinyUdpFlag f){
    p->flag = f;
}
void TinyUdp::SetSeq(TinyUdpPacket *p, char seq){
    p->seq = seq;
}
void TinyUdp::CalcChecksum(TinyUdpPacket *p){
    p->chechsum = 0;
    char sum = 0;
    char *c = (char *)(p);
    for (int i = 0; i<sizeof(TinyUdpPacket); i++){
        sum += *c;
    }
    sum = ~sum;
    p->chechsum = sum;
}
void TinyUdp::SetPayload(TinyUdpPacket *p, char *payload, int length){
    memset(p->payload, 0, 20);
    memcpy(p->payload, payload, 20);
}


RoutingTable::RoutingTable(){
    memset(routes, 0, sizeof(routes));
}

TinyNet::TinyNet(Address addr){
    myAddress = addr;
    RoutingTable *table = new RoutingTable(); //allocate on heap
    routes = table;
}

bool TinyNet::canRead(){
    return recvQueue.size()!=0;
}

bool TinyNet::canSend(Address dst){
    Hops dstRoute = routes->GetRoute(dst, 0);
    Hops hops = Hops();
    memset(&hops, 0, sizeof(Hops));
    return memcmp(&dstRoute, &hops, sizeof(Hops))!=0;
}

void TinyNet::Send(char* payload, int length){
    TinyUdpPacket *udpp = new TinyUdpPacket();
    TinyUdp udpUtil = TinyUdp();
    udpUtil.SetPayload(udpp, payload, length>20?20:length);
    udpUtil.SetSeq(udpp, 0);
    udpUtil.SetFlag(udpp, TinyUdpFlag{});
    TinyIpPacket *ipp = new TinyIpPacket();
    TinyIp ipUtil = TinyIp();
}
