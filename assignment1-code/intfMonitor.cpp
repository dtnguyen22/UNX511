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
#include "Interface.h"
//print interface stats
static void signalHandler(int sig);
void setInterfaceUp(string interfaceName, int sockfd);


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
        cout << getpid() << " intfMonitor: ready is sent" << endl;
    }
    memset(buf, 0, sizeof(buf)); //clean buf
    ret = read(fd, buf, BUF_LEN);
    if (strcmp(buf, "Monitor") == 0)
    {
        memset(buf, 0, sizeof(buf));            //clean buf
        len = sprintf(buf, "%s", "Monitoring"); //tell networkMonitor its ready to print stats
        write(fd, buf, BUF_LEN);
        isRunning = true;
        if (ret > 0)
        {
            cout << getpid() << " intfMonitor: Monitoring is sent" << endl;
        }
        /////////////////////////////////////
        //non blocking using timeout
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
        ///////////////////////////

        Interface interfaceObject; //contains interface information
        while (isRunning)
        {
            //prints interface stats
            interfaceObject.printInterfaceStats(interface);
            //checks if interface is down
            if (interfaceObject.operstate.compare("down") == 0)
            {
                memset(buf, 0, sizeof(buf));           //clean buf
                len = sprintf(buf, "%s", "Link Down"); //send to network
                ret = write(fd, buf, BUF_LEN);
                if (ret > 0)
                {
                    cout << getpid() << " intfMonitor: Link Down is sent" << endl;
                }
                //checks if the server asks intfMonitor to turn it up
                memset(buf, 0, sizeof(buf)); //clean buf
                ret = read(fd, buf, BUF_LEN);
                if (strcmp(buf, "Set Link Up") == 0)
                { //xecute an IOCTL command to set the link back up
                    cout << "intfMonitor: Set Link Up is received" << endl;
                    setInterfaceUp(interface, fd);
                }
            }
        }
    }

    cout << "intfMonitor: close(fd) before exitting" << endl;
    close(fd);
    return 0;
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

void setInterfaceUp(string interfaceName, int sockfd)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof ifr);
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ);
    ifr.ifr_flags |= IFF_UP;
    int ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    cout << "IOCTL returns: " << ret << endl;
    close(sockfd);
}
