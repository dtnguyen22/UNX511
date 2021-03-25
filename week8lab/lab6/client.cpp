#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

char socket_path[] = "/tmp/lab6";

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    char buf[100];
    int fd, rc, len;
    bool isRunning = true;

    memset(&addr, 0, sizeof(addr));
    //Create the socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        cout << "client: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    //Connect to the local socket
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cout << "client: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    //Read from the standard input (keyboard)
    while (isRunning)
    {
        rc = read(fd, buf, sizeof(buf));
        if (strncmp(buf, "Pid", 4) == 0)
        {
            cout << "A request for client id has been received" << endl;
            len = sprintf(buf, "%d", getpid()) + 1;
            rc = write(fd, buf, len);
            if (rc == -1)
            {
                cout << "client(" << getpid() << "): Write Error" << endl;
                cout << strerror(errno) << endl;
                close(fd);
                exit(-1);
            }
        }
        if (strncmp("Sleep", buf, 6) ==0)
        {
            cout << "This client is going to sleep for 5 seconds" << endl;
            sleep(5);
            len = sprintf(buf, "%s", "Done");
            rc = write(fd, buf, len);
            if (rc == -1)
            {
                cout << "client(" << buf << "): Write Error" << endl;
                cout << strerror(errno) << endl;
                close(fd);
                exit(-1);
            }
        }
        if (strncmp("Quit", buf, 5) == 0)
        {
            cout << "This client is quitting" << endl;
            isRunning = false;
        }
    }

    cout << "client: close(fd)" << endl;
    close(fd);
    return 0;
}
