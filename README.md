# Architecture

The object types in this library shown below.

- TinyNet

Similar to network interface. It has one IP address and contains all connections using this address. In addition to this, for hopping, it has a routing table refreshed automatically by received RIP packet.

- TinyConnection

Similar to socket in TCP socket communication. It have a port number and TinyNet move packet from its central queue where all packets are stored firstly, to the queue of connection using its port number.

You must create 3 threads for sending, receiving and handling packets.

- sending thread

This thread move packets from central sending queue which TinyNet has to next hop via something to transfer, such as BLE.

- receiving thread

This thread move packets from something to transfer to central received queue which TinyNet has.

- handling thread

This thread move packets from central received queue to received queue per connections.

# Internal Architecture

Tiny mesh has 2 incoming queue and 2 outgoing queue internally.

One queue is in TinyConnection. When string data is sent to someone, packet created from string data moves to in queue of TinyConnection.

When TinyNet::handleAllSendingPackets is called, packets in the queue move to queue of TinyNet.

User can pop packets from queue of TinyNet and send by something such as bluetooth.

When you receive packets, you can push to incoming queue of TinyNet.

When TinyNet::handleAllReceivedPackets is called, packets in queue of TinyNet classified by port number and protocol number.

Tiny RIP packets are handled in protocol stack. Tiny UDP packets will send to appropriate connections.

# How to send packets to remote host

First, create network, handling all received packets, connections and routing table.

```c
TinyNet *n = new TinyNet(/*IP Address of this machine*/);
```

Next, create a UDP connection. This layer covers packet size and division.

```c
TinyUdpPortNumber portNum = TinyUdpPortNumber{/*src port number*/,/*dst port number*/};
TinyConnection* con = n->InitConnection(portNum, /*dst address*/);
```

The next step is sending data. In fact, this step just push to sending queue of connection.

Note: packet division and automated re-request is under construction. Therefore, you have to make packets one by one.

```c
char data[TINYUDP_PAYLOAD_MAX] = "aiueo";
con->Send(n->GetRoute(), data, TINYUDP_PAYLOAD_MAX);
```

Push a packet to central sending queue from connection queue and send a packet stored in central sending queue via BLE.

Note: This can be divided to sending thread.

```c
n->handleAllSendingPackets(routes);
BLESend(getPacketFromCentralSendingQueue());
```

Receive a packet from BLE.

Note: this can be divided to receiving thread.

```c
TinyIpPacket p = BLEReceive();
n.addPacketToCentralReceivingQueue(p);
```

Handle all packet, moving from central receiving queue to queue of connection.

```c
n->handleAllReceivedPackets(routes);
```

After these steps, you can receive packets from connection.

```c
TinyIpPacket p = con->getPacketFromReceivedQueue();
```


