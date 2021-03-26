#ifndef INTERFACE_H
#define INTERFACE_H

#include <iostream>
#include <string>
// describes an interface
// interface.h
using namespace std;

class Interface{
public:
    string operstate;           //the operating state
    int carrier_up_count = 0;   //no of times the intf has been up
    int carrier_down_count = 0; //no of times the intf has been down
    int rx_bytes = 0;           //no of received bytes
    int rx_dropped = 0;         //no of dropped received bytes
    int rx_errors = 0;          //no of erroneous received bytes
    int rx_packets = 0;         //no of received packets
    int tx_bytes = 0;           //no of transmitted bytes
    int tx_dropped = 0;         //no of dropped transmitted bytes
    int tx_errors = 0;          //no of erroneous transmitted bytes
    int tx_packets = 0;         //no of transmitted packets
                                /////////////////////////////////////////////////////////////////

    void printInterfaceStats(const char interface[]);

    //read from file
    // string readInterfaceStat(string filePath);
};
#endif