#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

//print interface stats
void printInterfaceStats(const char interface[]);
static void signalHandler(int sig);

using namespace std;

char socket_path[] = "/tmp/assignment1";

bool isRunning = true;
const int BUF_LEN = 512;

int main(int argc, char *argv[])
{
    char interface[BUF_LEN];
    //interface name comes from first argument
    strncpy(interface, argv[1], BUF_LEN);

    //////////////////////////////////////////////////////////////////
    //Set up a signal handler to terminate the program gracefully
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    //Set up socket communications
    struct sockaddr_un addr;
    char buf[BUF_LEN];
    int len, ret;
    int fd, rc;
    cout << "intfMonitor (" << getpid() << ") is running..." << endl;
    memset(&addr, 0, sizeof(addr));
    //Create the socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        cout << "intfMonitor (" << getpid() << "): " << strerror(errno) << endl;
        exit(-1);
    }
    addr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    //Connect to the local socket
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cout << "client(" << getpid() << "): " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }
    /////////////////////////////////////////////////////////////////////////

    len = sprintf(buf, "%s", "Ready"); //tell networkMonitor its ready to print stats
    ret = write(fd, buf, BUF_LEN);
    if (ret > 0)
    {
        cout <<getpid()<<" intfMonitor: ready is sent" << endl;
    }
    memset(buf, 0, sizeof(buf));//clean buf
    ret = read(fd, buf, BUF_LEN);
    if (strcmp(buf, "Monitor") == 0)
    {
        while (isRunning)
        {
            memset(buf, 0, sizeof(buf));            //clean buf
            len = sprintf(buf, "%s", "Monitoring"); //tell networkMonitor its ready to print stats
            write(fd, buf, BUF_LEN);
            //print stats
            if (ret > 0)
            {
                cout <<getpid()<< " intfMonitor: Monitoring is sent" << endl;
            }
            printInterfaceStats(interface); //this func is a loop
        }                                   //0 means they are equal
    }
    if (strcpy(buf, "Set Link Up") == 0)
    { //xecute an IOCTL command to set the link back up
    }

    cout << "intfMonitor: close(fd) before exitting" << endl;
    close(fd);
    return 0;
}

//print interface stats
void printInterfaceStats(const char interface[])
{
    char statPath[2 * BUF_LEN];

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

    ifstream infile;

    //operate state
    sprintf(statPath, "/sys/class/net/%s/operstate", interface);
    infile.open(statPath);
    if (infile.is_open())
    {
        infile >> operstate;
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
    sleep(2);
}

static void signalHandler(int sig)
{
    switch (sig)
    {
    case SIGINT:
        cout << "intfMonitor: ctrl-C caught, exiting ..." << endl;
        isRunning = false;
        break;
    default:
        cout << "intfMonitor: undefined signal" << endl;
    }
}