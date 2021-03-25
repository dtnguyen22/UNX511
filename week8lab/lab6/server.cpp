#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

using namespace std;

char socket_path[] = "/tmp/lab6";

int main(int argc, char *argv[])
{
    struct sockaddr_un addr; //server
    char buf[100];
    int fd, clientFd, rc;
    bool isRunning = true;

    memset(&addr, 0, sizeof(addr));
    //Create the socket to handle the server socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        cout << "server: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX; //set server socket type AF_UNIX
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1); //set server socket path
    unlink(socket_path); //clear existed socket path

    //Bind the socket to this local socket file
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    { //
        cout << "server: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    //Listen for a client to connect to this local socket file
    if (listen(fd, 5) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    }else{
        cout << "server: waiting for the client..." << endl;
    }


    //Accept the client's connection to this local socket file
    if ((clientFd = accept(fd, NULL, NULL)) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    }else{
            cout<< "client connected to the server" << endl;
            cout << "server: accept()" << endl;
    }

    //requests client's pid
    cout << "The server requests the client's pid" << endl;
    int len = sprintf(buf, "%s", "Pid");
    rc = write(clientFd, buf, sizeof(buf));
    if (rc == -1)
    {
        cout << "server: request client pid Write Error" << endl;
        cout << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    //receives client's pid
    rc = read(clientFd, buf, sizeof(buf));
    if(rc > 0){
        cout << "server: This client has pid " << buf << endl;
    }

    //tell the client to sleep for 5 seconds
    len = sprintf(buf, "%s", "Sleep");
    cout << "The server requests the client to sleep" << endl;
    rc = write(clientFd, buf, sizeof(buf));
    if (rc == -1)
    {
        cout << "server: request client to sleep Write Error" << endl;
        cout << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    //waits for "Done" signal from client
    rc = read(clientFd, buf, sizeof(buf));
    if(rc > 0){
        if(strncmp(buf, "Done", 4) == 0){                     
            cout << "The server requests the client to quit" << endl;
            //tells the client to shutdown
            len = sprintf(buf, "%s", "Quit");
            rc = write(clientFd, buf, sizeof(buf));
            if (rc == -1)
            {
                cout << "server: request client to quit Write Error" << endl;
                cout << strerror(errno) << endl;
                close(fd);
                exit(-1);
            }
        }
    }

    cout << "server: close(fd), close(cl)" << endl;
    //remove socket file
    unlink(socket_path);
    //close socket connections
    close(fd);
    close(clientFd);
    return 0;
}
