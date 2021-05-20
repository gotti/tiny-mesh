#pragma once
#include <queue>
#include <vector>
#include <mutex>

/*
 * Address specification:
 * You can use 5-bits of char from MSB, the remaining 3-bits are reserved for other use, such as hop-by-hop options.
 * 00000 is loopback address.
 * 11111 is broadcast address.
 * others can be used for any purpose.
 */

typedef unsigned char Address;

/*
 * tiny-ip flag specification:
 * nhops is hop count, similar to ttl. tiny-rip supports 4-hops in default. (you can easily extend this limit.)
 * protocol is next protocol number.
 * when autoroute==1, relay node determine next hop automatically by its routing table, without using TinyIpPacket.hops
 * unreachable is similar to ICMP unreachable. tiny-icmp is not exist, however, this role is embedded in tiny-ip.
 * echorequest is similar to ICMP echo request. (used by ping).
 */
/*
 * protocol number specification in IpFlag
 * 00x reserved
 * 010 tiny-rip
 * 011 tiny-udp
 * 100 raw
 * 1xx reserved
 *
 * role of icmp is embedded in tiny-ip flag.
 */

struct IpFlag {
    char nhops:2;
    char protocol:3;
    char autoroute:1;
    char unreachable:1;
    char echorequest:1;
};

/*
 * tiny-ip header specification:
 * hops is used for static routing. When autoroute is set, sender can determine relay node when sending a packet.
 * When autoroute is not set, relay node add "next hop's address" to this section therefore receiver can track packet's route.
 */

struct TinyIpPacket{
    IpFlag flag;
    Address dst;
    Address hops[4];
    Address src;
    char payload[24];
};


class TinyIp {
public:
    void SetFlag(TinyIpPacket *p, IpFlag f);
    void SetDst(TinyIpPacket *p, Address dst);
    void SetSrc(TinyIpPacket *p, Address src);
    void SetHop(TinyIpPacket *p, Address hop[4]);
    void SetPayload(TinyIpPacket *p, char* payload, int length);
};

struct TinyUdpFlag {
    char requestResend:1;
    char reserved:7;
};

struct TinyUdpPacket {
    TinyUdpFlag flag;
    Address seq;
    Address chechsum;
    char payload[20];
};

class TinyUdp {
public:
    void SetFlag(TinyUdpPacket *p, TinyUdpFlag f);
    void SetSeq(TinyUdpPacket *p, char seq);
    void SetPayload(TinyUdpPacket *p, char* payload, int length);
    void CalcChecksum(TinyUdpPacket *p);
};

struct Hops{
    Address hop[4];
};

struct TinyRipFlag{
    char nhops:2;
    char advertise:1;
    char request:1;
    char responce:1;
    char reserved:3;
};

struct TinyRipPacket{
    TinyRipFlag flag;
    Hops hops;
};

/*
 * tiny-rip protocol specification:
 * tiny-rip generally receive and send broadcast packet. For avoiding packet loop, incrementing nhops and adding all relay nodes' address to hops are needed
 * tiny-rip is to determine routes. In tiny-rip, node have a routing table, which contains list of nodes.
 * Example:
 *   A-B-C-D
 *   \___/
 *
 *   In this example, D can send a packet in 2 routes. D→C→B→A or D→C→A.
 *   Routing table contains these data, routing table of node D is shown below.
 *   RoutingTable.Address[A][0]={C,0,0,0}
 *   RoutingTable.Address[A][1]={C,B,0,0}
 *   0 normally means broadcast, however in tiny-rip, 0 means no host.
 *   RoutingTable.Address[A] should be sorted by length(hops), shortest first.
 *   Since tiny-rip supports static routing by setting relay nodes' address when sending a packet, sender can determine which relay to use.
 *   If unreachable packet is received, node should destroy all routes for that address and resend in dynamic routing, or relay unreachable packet to receiver in static routing.
 *
 * packets for creating routing table
 * All nodes should advertise at regular intervals.
 * All nodes receiving advertisement add next hop's address to hops, re-advertise, and add route to routing table, unless hops already has their own address or nhops==4.
 * If node want to know route of destination address, node can send request packet. If certain node's address matches request packet's dst, the node should send responce packet setting its address to src.
 */

class TinyRip{
    void AddMyAddressToAdvertisement(TinyRipPacket *p, Address src);
    void MakeRequest(TinyRipPacket *p);
    bool CheckRequested(TinyRipPacket *p, Address dst);
};

struct Routes{
    Hops hop;
};

class RoutingTable{
    Hops routes[32][3];
public:
    RoutingTable();
    void AddRoute(Address dst, Address hop[4]);
    Hops GetRoute(Address dst, int index);
    Address GetNextHop(Address dst, int index);
    void DelRoute(Address dst, int index);
};


class TinyConnection{
    std::queue<TinyIpPacket> sendQueue;
    std::queue<TinyIpPacket> recvQueue;
public:
};

class PortTable{
    Address table[32];
public:
    PortTable();
    void AddRoute(int virtualPort, int address);
    void DelRoute(int virtualPort);
    Address GetRoute(int virtualPort);
};

class TinyNet{
    std::mutex mtx_;
    Address myAddress;
    RoutingTable *routes;
    PortTable *portTable;
    std::vector<TinyConnection*> connections;
public:
    TinyNet(Address myAddress);
    Address InitConnection();
    TinyConnection* GetConnection(Address ConNumber);
    void Send(char* payload, int length);
};
