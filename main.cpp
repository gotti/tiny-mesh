#include <string.h>
#include "main.hpp"

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
