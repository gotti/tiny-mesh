#include <string.h>
#include <mutex>
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

TinyConnection* TinyNet::InitConnection(Address dst){
    std::lock_guard<std::mutex> lock(mtx_);
    TinyConnection *conn = new TinyConnection{};
    connections.push_back(conn);
    conn->src = myAddress;
    conn->dst = dst;
    return conn;
};

RoutingTable* TinyNet::GetRoutes(){
    return routes;
}

void TinyConnection::Send(RoutingTable* routes, char* payload, int length){
    TinyUdpPacket *udpp = new TinyUdpPacket();
    TinyUdp::SetPayload(udpp, payload, length>20?20:length);
    TinyUdp::SetSeq(udpp, 0);
    TinyUdp::SetFlag(udpp, TinyUdpFlag{});

    TinyIpPacket *ipp = new TinyIpPacket();
    TinyIp::SetSrc(ipp, src);
    Hops hops = routes->GetRoute(dst, 0);
    TinyIp::SetDst(ipp, dst);
    TinyIp::SetFlag(ipp, IpFlag{.nhops = 0, .protocol = 0b011, .autoroute = 0, .unreachable = 0, .echorequest = 0});
    TinyIp::SetPayload(ipp, (char*)udpp, 24);
    TinyIp::SetHop(ipp, hops);

    std::lock_guard<std::mutex> lock(sendmtx);
    sendQueue.push(*ipp);
}
