#include "net.hpp"
//#include "net.cpp"

int main(){
    TinyNet *n = new TinyNet(30);
    TinyConnection* con = n->InitConnection(29);
    con->Send();
}
