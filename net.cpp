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
}

Hops RoutingTable::GetRoute(Address dst, int index){
    if (0 <= index && index <= 31){
        return routes[dst][index];
    }else{
        return Hops();
    }
}

Address RoutingTable::GetNextHop(Address dst, int index, unsigned char nhops){
    //TODO: array size check
    return routes[dst][index].hop[nhops];
}

void RoutingTable::dumpRoutingTable(){
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%2d: ", i);
            printf("%d [hops] ", j);
            for (int k = 0; k < 4; k++) {
                printf("%d ", routes[i][j].hop[k]);
            }
            printf("\n");
        }
    }
}

TinyNet::TinyNet(Address addr){
    myAddress = addr;
    RoutingTable *table = new RoutingTable(); //allocate on heap
    routes = table;
}

void TinyNet::addPacketToCentralReceivingQueue(TinyIpPacket p){
    std::lock_guard<std::mutex> lock(recvmtx);
    recvQueue.push(p);
}

TinyIpPacket TinyNet::getPacketFromCentralSendingQueue(){
    std::lock_guard<std::mutex> lock(sendmtx);
    TinyIpPacket p = sendQueue.front();
    sendQueue.pop();
    return p;
}

TinyConnection* TinyNet::InitConnection(TinyUdpPortNumber portNum, Address dst){
    std::lock_guard<std::mutex> lock(mtx_);
    TinyConnection *conn = new TinyConnection{};
    connections.push_back(conn);
    conn->src = myAddress;
    conn->dst = dst;
    conn->portNum = portNum;
    enabledConnectionNumber[portNum.srcPort] = true;
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

void TinyIp::hex_dmp(TinyIpPacket *p){
    hex_dmp(p, sizeof(TinyIpPacket));
}

void TinyConnection::Send(RoutingTable* routes, char* payload, int length){
    TinyUdpPacket *udpp = new TinyUdpPacket();
    TinyUdp::SetPayload(udpp, payload, length);
    TinyUdp::SetSeq(udpp, 0);
    TinyUdp::SetFlag(udpp, TinyUdpFlag{});

    TinyIpPacket *ipp = new TinyIpPacket();
    TinyIp::SetSrc(ipp, src);
    Hops hops = routes->GetRoute(dst, 0);
    TinyIp::SetDst(ipp, dst);
    TinyIp::SetFlag(ipp, IpFlag{.nhops = 0, .protocol = 0b011, .autoroute = 0, .unreachable = 0, .echorequest = 0});
    TinyIp::SetPayload(ipp, (char*)udpp, TINYIP_PAYLOAD_MAX);
    TinyIp::SetHop(ipp, hops);

    std::lock_guard<std::mutex> lock(sendmtx);
    printf("dst: %d\n",ipp->dst);
    sendQueue.push(*ipp);
}

void TinyConnection::AddPacketToQueue(TinyIpPacket p){
    std::lock_guard<std::mutex> lock(recvmtx);
    recvQueue.push(p);
}

TinyIpPacket TinyConnection::getPacketFromReceivedQueue(){
    //途中
    std::lock_guard<std::mutex> lock1(recvmtx);
    TinyIpPacket p = recvQueue.front();
    recvQueue.pop();
    return p;
}
TinyUdpPacket TinyConnection::getTinyUdpPacket(){
    TinyIpPacket p = getPacketFromReceivedQueue();
    TinyUdpPacket *udpp = (TinyUdpPacket*)(p.payload);
    return *udpp;
}

TinyIpPacket TinyConnection::getPacketFromSendingQueue(){
    //途中
    std::lock_guard<std::mutex> lock1(sendmtx);
    TinyIpPacket p = sendQueue.front();
    sendQueue.pop();
    return p;
}

//実装考え中
void TinyNet::handleAllSendingPackets(RoutingTable *routes){
    std::lock_guard<std::mutex> lock1(sendmtx);
    for (int i = 0; i < 32; i++) {
        if(enabledConnectionNumber[i]){
            TinyIpPacket p = connections.at(i)->getPacketFromSendingQueue();
            sendQueue.push(p);
        }
    }
}

void TinyNet::handleAllReceivedPackets(RoutingTable* routes){
    printf("packets: %ld\n", recvQueue.size());
    if (recvQueue.size() <= 0){
        return;
    }
    printf("front:");
    std::lock_guard<std::mutex> lock1(sendmtx);
    std::lock_guard<std::mutex> lock2(recvmtx);
    TinyIpPacket ipp = recvQueue.front();
    recvQueue.pop();
    // If received packets is for me
    printf("handling p:%d\n",ipp.flag.protocol);
    if (ipp.dst == myAddress) {
        switch (ipp.flag.protocol){
            //tine rip
            case PROTOCOL_TINY_RIP:
                routes->RefreshRoutingTable(ipp);
                return;
            case PROTOCOL_TINY_UDP:
                TinyUdpPacket p = *(TinyUdpPacket *)(ipp.payload);
                if (enabledConnectionNumber[p.portNum.dstPort>31?31:p.portNum.srcPort]){
                    connections.at(p.portNum.srcPort)->AddPacketToQueue(ipp);
                }
                return;
        }
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
            ipp.hops[ipp.flag.nhops] = routes->GetNextHop(ipp.dst, 0, ipp.flag.nhops);
            sendQueue.push(ipp);
            return;
        }
    }
}


void RoutingTable::RefreshRoutingTable(TinyIpPacket p){
    std::lock_guard<std::mutex> lock(mtx);
    TinyRipPacket ripp = *(TinyRipPacket *)(p.payload);
    printf("refreshing");
    for (int i=0; i<3; i++){
        if (routes[p.src][i].hop[0]==0){
            memcpy(routes[p.src][0].hop,&(ripp.hops),sizeof(Hops));
            return;
        }
    }
}
