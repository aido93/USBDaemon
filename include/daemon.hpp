#ifndef __DAEMON_CPP
#define __DAEMON_CPP

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <execinfo.h>
#include <string>
#include "usb_daemon_conf.h"
#include "cfunctions.h"

using namespace std;

class TDaemon{
    protected:
        static const unsigned int FD_LIMIT;
        static string PID_FILE;
        static string CONFIG_FILE;
    public:
        TDaemon(const char* ConfigName);
        ~TDaemon(){ };
        virtual int DaemonFunction() = 0;
        string GetConfigFilename(){return CONFIG_FILE;};
        void SetPidFile(const char* Filename);
        int MonitorProc();
        int SetFdLimit(int MaxFd);
        int WorkProc();
        int ReloadConfig();
        virtual int LoadConfig(const char* Filename) = 0;
        static void SelfDebug(int sig, siginfo_t *si, void *ptr);
};

#endif
