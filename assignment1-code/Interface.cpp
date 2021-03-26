#include <iostream>
#include <fstream>
#include <unistd.h>
#include "Interface.h"

void Interface::printInterfaceStats(const char interface[])
{
    char statPath[2 * 512];
    ifstream infile;

    //operate state
    sprintf(statPath, "/sys/class/net/%s/operstate", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> this->operstate;
        infile.close();
    }

    //carrier up count
    sprintf(statPath, "/sys/class/net/%s/carrier_up_count", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> carrier_up_count;
        infile.close();
    }

    //carrier down count
    sprintf(statPath, "/sys/class/net/%s/carrier_down_count", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> carrier_down_count;
        infile.close();
    }

    //no of received bytes
    sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> rx_bytes;
        infile.close();
    }

    //no of erroneous received bytes
    sprintf(statPath, "/sys/class/net/%s/statistics/rx_dropped", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> rx_dropped;
        infile.close();
    }

    //no of erroneous received bytes
    sprintf(statPath, "/sys/class/net/%s/statistics/rx_errors", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> rx_errors;
        infile.close();
    }

    //no of received packets
    sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> rx_packets;
        infile.close();
    }
    //no of transmitted bytes
    sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> tx_bytes;
        infile.close();
    }
    //no of dropped transmitted bytes
    sprintf(statPath, "/sys/class/net/%s/statistics/tx_dropped", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> tx_dropped;
        infile.close();
    }
    //no of erroneous transmitted bytes
    sprintf(statPath, "/sys/class/net/%s/statistics/tx_errors", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> tx_errors;
        infile.close();
    }
    //no of transmitted packets
    sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> tx_packets;
        infile.close();
    }

    cout << " Interface: " << interface << " state: " << operstate << " up_count: " << carrier_up_count << " down_count: " << carrier_down_count << endl;
    cout << " rx_bytes: " << rx_bytes << " rx_dropped: " << rx_dropped << " rx_errors: " << rx_errors << " rx_packets: " << rx_packets << endl;
    cout << " tx_bytes: " << tx_bytes << " tx_dropped: " << tx_dropped << " tx_errors: " << tx_errors << " tx_packets: " << tx_packets << endl
         << endl;
    sleep(1);
}
