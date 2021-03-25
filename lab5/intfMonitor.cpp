//intfMonitor_solution.cpp - An interface monitor
//
// 13-Jul-20  M. Watler         Created.

#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

const int MAXBUF=128;
bool isRunning=false;

//TODO: Declare your signal handler function prototype
static void sigHandler(int sig);

int main(int argc, char *argv[])
{
    //TODO: Declare a variable of type struct sigaction
    //      For sigaction, see http://man7.org/linux/man-pages/man2/sigaction.2.html
	struct sigaction action;
    char interface[MAXBUF];
    char statPath[MAXBUF];
    const char logfile[]="Network.log";//store network data in Network.log
    int retVal=0;

    //TODO: Register signal handlers for SIGUSR1, SIGUSR2, ctrl-C and ctrl-Z
    //TODO: Ensure there are no errors in registering the handlers
    action.sa_handler = sigHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
	int err1 = sigaction(SIGINT, &action, NULL);//sigaction return 0 on success
    int err3 = sigaction(SIGUSR1, &action, NULL);
	int err4 = sigaction(SIGUSR2, &action, NULL);
	int err2 = sigaction(SIGTSTP, &action, NULL);

	if((err1 + err2 + err3 + err4) != 0){ //sigaction return 0 on success
		cout << "Sigaction registration failed" << endl;
	}
// 	sighandler_t err1=signal(SIGINT, sigHandler);
// 	sighandler_t err2=signal(SIGUSR1, sigHandler);
// 	sighandler_t err3=signal(SIGUSR2, sigHandler);
// 	sighandler_t err4=signal(SIGTSTP, sigHandler);

//    if(err1==SIG_ERR || err2==SIG_ERR || err3==SIG_ERR || err4==SIG_ERR) {
//        cout<<"Cannot create the signal handler"<<endl;
//        cout<<strerror(errno)<<endl;
//        return -1;
//    }

    strncpy(interface, argv[1], MAXBUF);//The interface has been passed as an argument to intfMonitor
    int fd=open(logfile, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    cout<<"intfMonitor:main: interface:"<<interface<<":  pid:"<<getpid()<<endl;

    //TODO: Wait for SIGUSR1 - the start signal from the parent
	// isRunning = true;
    while(isRunning) {
        //gather some stats
        int tx_bytes=0;
        int rx_bytes=0;
        int tx_packets=0;
        int rx_packets=0;
	    ifstream infile;
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_bytes;
	        infile.close();
	    }
            sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_bytes;
	        infile.close();
	    }
            sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_packets;
	        infile.close();
	    }
            sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_packets;
	        infile.close();
	    }
	    char data[MAXBUF];
	    //write the stats into Network.log
	    int len=sprintf(data, "%s: tx_bytes:%d rx_bytes:%d tx_packets:%d rx_packets: %d\n", interface, tx_bytes, rx_bytes, tx_packets, rx_packets);
	    write(fd, data, len);
	    sleep(1);
    }
    close(fd);

    return 0;
}



static void sigHandler(int sig)
{
	switch(sig){
	case SIGUSR1:
        cout << "intfMonitor: ("<< getpid()<< ") starting up" << endl;
        isRunning = true;
        break;
	case SIGUSR2:
		cout << "processMonitor: ("<< getpid()<< ") SIGUSR 2 received" << endl;
		isRunning = false;
		break;
	case SIGINT:
		cout << "intfMonitor: ctrl-C discarded" << endl;
		break;
	case SIGTSTP:
		cout << "intfMonitor: ctrl-Z discarded" << endl;
		break;
	default:
		cout << "intfMonitor: undefined signal" << endl;
	}
}

