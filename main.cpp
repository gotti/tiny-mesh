#include "net.hpp"
//#include "net.cpp"

int main(){
    TinyNet *n = new TinyNet(31);
    Address portNum = n->InitConnection();
    TinyConnection* con = n->GetConnection(portNum);
}
