#include "net.hpp"
//#include "net.cpp"

void sendThread(TinyNet *n, RoutingTable *routes) {
  while (true) {
    n->handleAllSendingPackets(routes);
    break;
  }
}
void recvThread(TinyNet *n, RoutingTable *routes) {
  while (true) {
//    TinyRipPacket *ripp = new TinyRipPacket();
//    ripp->flag = TinyRipFlag();
//    ripp->hops = Hops{5, 4, 3, 2};
//    TinyIpPacket *p = new TinyIpPacket();
//    TinyIp::SetPayload(p, (char *)ripp, 99);
//    p->src = 30;
//    p->dst = 29;
//    p->flag = IpFlag{0, PROTOCOL_TINY_UDP, 0, 0, 0};
//    n->addPacketToCentralReceivingQueue(*p);
    break;
  }
}

void handleThread(TinyNet *n, RoutingTable *routes) {
  while (true) {
    n->handleAllReceivedPackets(routes);
    break;
  }
}

void sendThread29(TinyNet *n, TinyNet *n29, RoutingTable *routes) {}

int main() {
  //create TinyNet, something similar to network interface
  TinyNet *n = new TinyNet(30);
  TinyUdpPortNumber portNum = TinyUdpPortNumber{0, 0};
  TinyConnection *con = n->InitConnection(portNum, 29);

  //create TinyNet, something similar to network interface
  TinyNet *n29 = new TinyNet(29);
  TinyUdpPortNumber portNum29 = TinyUdpPortNumber{0, 0};
  TinyConnection *con29 = n29->InitConnection(portNum29, 30);

  //create udp packet and send message
  char data[TINYUDP_PAYLOAD_MAX] = "aiueo";
  con->Send(n->GetRoute(), data, TINYUDP_PAYLOAD_MAX);
  sendThread(n, n->GetRoute());
  handleThread(n, n->GetRoute());

  //receive udp packet
  n29->addPacketToCentralReceivingQueue(n->getPacketFromCentralSendingQueue());
  n29->handleAllReceivedPackets(n29->GetRoute());


  TinyIpPacket p = con29->getPacketFromReceivedQueue();
  TinyIp::hex_dmp(&p);
}
