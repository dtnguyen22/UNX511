//fileMon.cpp - log file monitor
//
// 11-Feb-21  M. Watler         Created.
//

#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "memoryUsage.h"

using namespace std;
bool isRunning;
const int MAXBUF = 64;
const int THRESHOLD = 40000;

static void sigHandler(int sig)
{
    switch (sig)
    {
    case SIGINT:
        cout << "fileMon: SIGINT received" << endl;
        isRunning = false;
        break;
    }
}

int main(int argc, char *argv[])
{
    char key1[MAXBUF], key2[MAXBUF];
    strncpy(key1, argv[1], MAXBUF);
    cout << "test argv[1]" << argv[1] << endl;
    int fd = open(keyFile, O_RDWR);
    read(fd, key2, MAXBUF);
    close(fd);
    //Question 18: Where is key1 coming from?
    //answer:
    //  fileMon is spawned by processMon using execlp(args[i], args[i], (const char *)key, NULL);
    //  and key which is retrieved from kernel module at /dev/keygen, is passed as parameter.
    //Question 19: Where is key2 coming from?
    //answer: key2  comes from kernel module at /dev/keygen
    //Question 20: Why are they being compared?
    //answer: it is to check the validity of these 2 keys, because they are retrieved the same way, their values must be the same
    int valid = strcmp(key1, key2);
    if (valid != 0)
    {
        cout << "Invalid key, aborting..." << endl;
    }
    else
    {
        struct sigaction action;
        action.sa_handler = sigHandler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGINT, &action, NULL);

        isRunning = true;
        struct stat st;
        while (isRunning)
        {
            stat(logFile, &st);
            if (st.st_size > THRESHOLD)
            {
                int openFlags = O_RDWR | O_CREAT | O_TRUNC;
                mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
                //Question 21: What do the following two lines effectively do to the log file?
                //answer: the only thing changed is the O_TRUNC flag, however,
                // it does not affect the content of the log file because it just opens, do nothing and closes the file.
                // and O_flags affect only the current open, not the file permission on the system.
                fd = open(logFile, openFlags, filePerms);
                close(fd);
            }
            sleep(1);
        }
    }
    return 0;
}
