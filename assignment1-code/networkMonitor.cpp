//server.cpp - Server code for multiple connections
//
// 06-Mar-20  M. Watler         Created.

#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

//function prototype
static void signalHandler(int signum);

using namespace std;

char socket_path[] = "/tmp/assignment1";
const int BUF_LEN = 100;
const int MAX_CLIENTS = 2;
bool is_running;

//for child process management
pid_t childPid[2];

char *intf[] = {"lo", "ens33"};

int main(int argc, char *argv[])
{
    //Create the socket for inter-process communications
    struct sockaddr_un addr;
    char buf[BUF_LEN];
    int len;
    int master_fd, max_fd, cl[MAX_CLIENTS], rc;
    fd_set active_fd_set;
    fd_set read_fd_set;
    int ret;
    int numClients = 0;

    //////////////////////////////////////////////////////////////////
    //Set up a signal handler to terminate the program gracefully
    // struct sigaction action;
    // action.sa_handler = signalHandler;
    // sigemptyset(&action.sa_mask);
    // action.sa_flags = 0;
    // int err1 = sigaction(SIGINT, &action, NULL);//sigaction return 0 on success
    // int err2 = sigaction(SIGTSTP, &action, NULL);
    // if((err1 + err2) != 0){ //sigaction return 0 on success
    // 	cout << "Sigaction registration failed" << endl;
    // }
    ///////////////////////////////////////////////////////////////////

    bool isParent = true;
    cout << endl
         << "Spawning intfMoniter processes" << endl;
    for (int i = 0; i < 2 & isParent; ++i)
    {
        childPid[i] = fork();
        if (childPid[i] == 0)
        { //the child
            cout << "child:main: pid:" << getpid() << endl;
            isParent = false;
            execlp("./intfMonitor", "./intfMonitor", intf[i], NULL);
            cout << "child:main: pid:" << getpid() << " I should not get here!" << endl;
            cout << strerror(errno) << endl;
        }
    }
    //Create the socket
    memset(&addr, 0, sizeof(addr));
    if ((master_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        cout << "server: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    unlink(socket_path);

    //Bind the socket to this local socket file
    if (bind(master_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        close(master_fd);
        exit(-1);
    }

    cout << "Waiting for the client..." << endl;
    //Listen for a client to connect to this local socket file
    if (listen(master_fd, 5) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(master_fd);
        exit(-1);
    }

    FD_ZERO(&read_fd_set); //set fd to all 0 bit
    FD_ZERO(&active_fd_set);
    //Add the master_fd to the socket set
    FD_SET(master_fd, &active_fd_set);
    max_fd = master_fd; //We will select from max_fd+1 sockets (plus one is due to a new connection)
    is_running = true;

    while (is_running)
    {
        //Block until an input arrives on one or more sockets
        read_fd_set = active_fd_set;
        ret = select(max_fd + 1, &read_fd_set, NULL, NULL, NULL); //Select from up to max_fd+1 sockets
        if (ret < 0)
        {
            cout << "server: " << strerror(errno) << endl;
        }
        else
        { //Service all the sockets with input pending
            if (FD_ISSET(master_fd, &read_fd_set))
            { //Connection request on the master socket
                cl[numClients] = accept(master_fd, NULL, NULL); //Accept the new connection
                if (cl[numClients] < 0)
                {
                    cout << "server: " << strerror(errno) << endl;
                }
                else
                {
                    cout << "Server: incoming connection " << cl[numClients] << endl;
                    FD_SET(cl[numClients], &active_fd_set); //Add the new connection to the set
                    memset(buf, 0, sizeof(buf));//clean buf
                    ret = read(cl[numClients], buf, BUF_LEN); //Read the data from that client
                    if(strcmp(buf, "Ready") == 0){ //0 means they are equal
                        cout << "networkMonitor: received Ready" <<endl;
                        memset(buf, 0, sizeof(buf));//clean buf
                        len = sprintf(buf, "%s", "Monitor"); //tell intfMonitor networkMonitor is ready
                        ret = write(cl[numClients], buf, len);
                    }
                    if(ret==-1) {
                        cout<<"server: Write Error"<<endl;
                        cout<<strerror(errno)<<endl;
                    }
                    if (max_fd < cl[numClients]){
                        max_fd = cl[numClients]; //Update the maximum fd
                    }
                    ++numClients;
                }
            }
            else //Data arriving on an already-connected socket
            {
                for (int i = 0; i < numClients; ++i)
                { //Find which client sent the data
                    if (FD_ISSET(cl[i], &read_fd_set))
                    {
                        memset(buf, 0, sizeof(buf));//clean buf
                        ret = read(cl[i], buf, BUF_LEN); //Read the data from that client
                        if(strcmp(buf, "Monitoring") == 0){ //0 means they are equal
                            cout << "Monitoring status received" << endl;
                        }
                        if (ret == -1)
                        {
                            cout << "server: Read Error" << endl;
                            cout << strerror(errno) << endl;
                        }
                        // cout << "server: read(sock:" << cl[i] << ", buf:" << buf << ")" << endl;
                    }
                }
            }
        }
    }

    for (int i = 0; i < numClients; ++i)
    { //Request each client to quit
        cout << "server: request client " << i + 1 << " to quit" << endl;
        len = sprintf(buf, "Quit") + 1;
        ret = write(cl[i], buf, len);
        if (ret == -1)
        {
            cout << "server: Write Error" << endl;
            cout << strerror(errno) << endl;
        }
        sleep(1);                      //Give the clients a change to quit
        FD_CLR(cl[i], &active_fd_set); //Remove the socket from the set of active sockets
        close(cl[i]);                  //Close the socket connection
    }

    //Close the master socket
    close(master_fd);
    //Remove the socket file from /tmp
    unlink(socket_path);
    return 0;
}

static void signalHandler(int sig)
{
    cout << "networkMonitor: signalHandler(" << sig << "): SIGINT" << endl;
    switch (sig)
    {
    case SIGINT:
        is_running=false;
        cout << "networkMonitor: signalHandler(" << sig << "): SIGINT" << endl;
        for (int i = 0; i < 2; i++)
        {
            kill(childPid[i], SIGINT);
        }
        break;
    default:
        cout << "networkMonitor: undefined signal" << endl;
    }
}