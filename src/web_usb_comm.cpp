
/**
 * 
 * This file needs to determine variables
 * 
 * */
#include <mutex>
#include <condition_variable>
#include "protocol.h"
#include <string>

//for rerunning any of the thread
pthread_cond_t *cond_web;
pthread_mutex_t *mut_web;
int WebServer_ToJoin;
pthread_cond_t *cond_usb;
pthread_mutex_t *mut_usb;
int USBHandler_ToJoin;

//thread-thread communication
std::mutex m1;
std::condition_variable cv1;
bool readyToRun = false;
bool received=true;
bool ready;
struct TPacket1 pkg;
std::string ans;
