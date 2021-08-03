typedef unsigned char Address;

struct Flag {
    unsigned char nhops:1;
    unsigned char reserved:5;
    unsigned char unreachable:1;
    unsigned char echorequest:1;
};
class TinyIp {
    Flag flag;
    Address dst;
    Address hop[4];
    Address src;
    char* payload;

public:
    void SetFlag(Flag f);
    void SetDst(Address dst);
    void SetSrc(Address src);
    void SetHop(Address hop[4]);
    void SetPayload(char* payload);
};

class TinyTCP {
    Address header;
    Address seq;
    Address chechsum;
    char* payload;
};

struct Hops{
    Address hop[4];
};

class TinyRIP{
    Address src;
    Hops hop;
};

struct Routes{
    Hops hop;
};

class RoutingTable{
    Address routes[32][3][4];
public:
    void AddRoute(Address dst, Address hop[4]);
    Hops GetRoute(Address dst, int index);
    Address GetNextHop(Address dst, int index);
    void DelRoute(Address dst, int index);
};
