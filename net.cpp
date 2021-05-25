#include <string.h>
#include <string>
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
void TinyIp::SetHop(TinyIpPacket *p, Hops hop){
    memcpy(p->hops, &hop, sizeof(Address)*4); //?
}
void TinyIp::SetPayload(TinyIpPacket *p, char* payload, int length){
    memset(p->payload, 0, TINYIP_PAYLOAD_MAX);
    memcpy(p->payload, payload, length>TINYIP_PAYLOAD_MAX ? TINYIP_PAYLOAD_MAX : length);
}
std::string TinyIp::print(TinyIpPacket *p){
    std::string ret;
//    ret += "src:" + std::to_string(p->src);
//    ret += "\ndst:" + std::to_string(p->dst);
//    ret += "\npayload:" + std::string(p->payload);
    return ret;
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
    memset(p->payload, 0, TINYUDP_PAYLOAD_MAX);
    memcpy(p->payload, payload, length>TINYUDP_PAYLOAD_MAX?TINYUDP_PAYLOAD_MAX:length);
}


RoutingTable::RoutingTable(){
    memset(routes, 0, sizeof(routes));
}

Hops RoutingTable::GetRoute(Address dst, int index){
    if (0 <= index && index <= 31){
        return routes[dst][index];
    }else{
        return Hops();
    }
}

TinyNet::TinyNet(Address addr){
    myAddress = addr;
    RoutingTable *table = new RoutingTable(); //allocate on heap
    routes = table;
}

TinyConnection* TinyNet::InitConnection(TinyUdpPortNumber portNum, Address dst){
    std::lock_guard<std::mutex> lock(mtx_);
    TinyConnection *conn = new TinyConnection{};
    connections.push_back(conn);
    conn->src = myAddress;
    conn->dst = dst;
    conn->portNum = portNum;
    return conn;
};

RoutingTable* TinyNet::GetRoute(){
    return routes;
}

// from https://qiita.com/tk23ohtani/items/4d344db7375c8d96472b
void hex_dmp(const void *buf, int size)
{
    int i,j;
    unsigned char *p = (unsigned char *)buf, tmp[20];

    printf("+0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F|  -- ASCII --\r\n");
    printf("--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+----------------\r\n");
    for (i=0; p-(unsigned char *)buf<size; i++) {
        for (j=0; j<16; j++) {
            tmp[j] = (unsigned char)((*p<0x20||*p>=0x7f)? '.': *p);
            printf("%02X ", (int)*p);
            if (++p-(unsigned char *)buf>=size) {
                tmp[++j] = '\0';
                for (;j<16;j++) {
                    printf("   ");
                }
                break;
            }
        }
        tmp[16] = '\0';
        printf("%s\r\n", tmp);
        if (p-(unsigned char *)buf>=size) {
            break;
        }
    }
}

void TinyConnection::Send(RoutingTable* routes, char* payload, int length){
    TinyUdpPacket *udpp = new TinyUdpPacket();
    TinyUdp::SetPayload(udpp, payload, length);
    TinyUdp::SetSeq(udpp, 0);
    TinyUdp::SetFlag(udpp, TinyUdpFlag{});
    hex_dmp(udpp, TINYUDP_PAYLOAD_MAX);

    TinyIpPacket *ipp = new TinyIpPacket();
    TinyIp::SetSrc(ipp, src);
    Hops hops = routes->GetRoute(dst, 0);
    TinyIp::SetDst(ipp, dst);
    TinyIp::SetFlag(ipp, IpFlag{.nhops = 0, .protocol = 0b011, .autoroute = 0, .unreachable = 0, .echorequest = 0});
    TinyIp::SetPayload(ipp, (char*)udpp, TINYIP_PAYLOAD_MAX);
    TinyIp::SetHop(ipp, hops);
    hex_dmp(ipp, TINYIP_PAYLOAD_MAX);

    std::lock_guard<std::mutex> lock(sendmtx);
    printf("%s\n",TinyIp::print(ipp).c_str());
    sendQueue.push(*ipp);
}

//実装考え中
void TinyNet::HandleAllPackets(RoutingTable* routes){
    std::lock_guard<std::mutex> lock1(sendmtx);
    std::lock_guard<std::mutex> lock2(recvmtx);
    TinyIpPacket ipp = recvQueue.front();
    recvQueue.pop();
    // If received packets is for me
    if (ipp.dst == src) {
        std::lock_guard<std::mutex> lock(usermtx);
        userQueue.push(ipp);
        return;
    }
    if (ipp.hops[ipp.flag.nhops]){
        // destroy a packet for tll
        if (ipp.flag.nhops == 3){
            return;
        }
        // handle static routing packet
        if (ipp.flag.autoroute == 0){
            ipp.flag.nhops++;
            sendQueue.push(ipp);
            return;
        // handle dynamic routing packet
        } else {
            ipp.flag.nhops++;
            ipp.hops[ipp.flag.nhops] = routes->GetNextHop(ipp.dst, 0);
            sendQueue.push(ipp);
            return;
        }
    }
}

void RoutingTable::RefreshRoutingTable(){
}
