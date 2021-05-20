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

PortTable::PortTable(){
    memset(table, 0, sizeof(table));
}

TinyNet::TinyNet(Address addr){
    myAddress = addr;
    RoutingTable *table = new RoutingTable(); //allocate on heap
    routes = table;
    PortTable *porttable = new PortTable();
    portTable = porttable;
}

Address TinyNet::InitConnection(){
    std::lock_guard<std::mutex> lock(mtx_);
    TinyConnection *conn = new TinyConnection{};
    connections.push_back(conn);
    size_t s = connections.size();
    return s-1;
};

TinyConnection* TinyNet::GetConnection(Address conNumber){
    return connections.at(conNumber);
}

void TinyNet::Send(char* payload, int length){
    TinyUdpPacket *udpp = new TinyUdpPacket();
    TinyUdp udpUtil = TinyUdp();
    udpUtil.SetPayload(udpp, payload, length>20?20:length);
    udpUtil.SetSeq(udpp, 0);
    udpUtil.SetFlag(udpp, TinyUdpFlag{});
    TinyIpPacket *ipp = new TinyIpPacket();
    TinyIp ipUtil = TinyIp();
    ipUtil.SetSrc(ipp, myAddress);
}
