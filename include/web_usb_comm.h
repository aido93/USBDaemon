#ifndef __WEB_USBCOMMUNICATION
#define __WEB_USBCOMMUNICATION

#include <string>
#include "protocol.h"
#include "web_usb_manage.h"

//thread-thread communication
extern std::mutex m1;
extern std::condition_variable cv1;
extern bool received;
extern bool ready;
extern struct TPacket1 pkg;
extern std::string ans;

#define CHILD_NEED_TERMINATE 1
#define CHILD_NEED_WORK 2

#define EPROCESS_EXISTS 3
#endif
