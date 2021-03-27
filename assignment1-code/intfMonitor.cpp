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
#include "Interface.h"//contains interface stats

//print interface stats
static void signalHandler(int sig);

//use ioctl to bring the interface up
int set_if_up(char *ifname, short flags);
int set_if_flags(char *ifname, short flags);

//
void writeMessage(int fd, string message);


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
    #ifdef DEBUG
    cout << "intfMonitor (" << getpid() << ") is running..." << endl;
    #endif
    memset(&addr, 0, sizeof(addr));
    //Create the socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
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

    /////////////////////////////////////
    //non blocking using timeout
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    ///////////////////////////

    writeMessage(fd, "Ready");//tells the networkMonitor it is ready

    memset(buf, 0, sizeof(buf)); //clean buf
    ret = read(fd, buf, BUF_LEN);//receives message from network monitor
    if (strcmp(buf, "Monitor") == 0)
    {
        writeMessage(fd, "Monitoring");//sends Monitoring to networkMonito
        isRunning = true;
        Interface interfaceObject; //contains interface information
        interfaceObject.name = interface;
        while (isRunning)
        {
            //prints interface stats
            interfaceObject.printInterfaceStats(interface);
            //checks if interface is down
            if (interfaceObject.operstate.compare("down") == 0)
            {
                writeMessage(fd, "Link Down");//tells networkMonitor the interface is down
                //checks if the server asks intfMonitor to turn it up
                memset(buf, 0, sizeof(buf)); //clean buf
                ret = read(fd, buf, BUF_LEN);
                if (strcmp(buf, "Set Link Up") == 0){ 
                    cout << "intfMonitor: Set Link Up is received" << endl;
                    //xecute an IOCTL command to set the link back up
                    set_if_up(interface, 1);
                }
            }
        }
    }

    cout << "intfMonitor: close(fd) before exitting" << endl;
    close(fd);//close fd socket
    unlink(socket_path);//remove the socket file
    return 0;
}

void writeMessage(int fd, string message){
    char buf[512];
    int len, ret;
    memset(buf, 0, sizeof(buf));//clean buf
    len = sprintf(buf, "%s", message.c_str()); //send to network
    ret = write(fd, buf, BUF_LEN);
    #ifdef DEBUG
    if (ret > 0){
        cout << getpid() << " intfMonitor: "<<message<<" is sent" << endl;
    }
    #endif
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

int set_if_flags(char *ifname, short flags)
{
    int skfd = -1;      /* AF_INET socket for ioctl() calls.*/
    struct ifreq ifr;
    int res = 0;

    ifr.ifr_flags = flags;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket error %s\n", strerror(errno));
        res = 1;
        goto out;
    }

    res = ioctl(skfd, SIOCSIFFLAGS, &ifr);
    #ifdef DEBUG
    if (res < 0) {
        printf("Interface '%s': Error: SIOCSIFFLAGS failed: %s\n",
            ifname, strerror(errno));
    } else {
        printf("Interface '%s': flags set to %04X.\n", ifname, flags);
    }
    #endif
out:
    close(skfd);
    return res;
}
int set_if_up(char *ifname, short flags)
{
    return set_if_flags(ifname, flags | IFF_UP);
}