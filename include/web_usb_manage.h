#ifndef __WEB_USB_MANAGING
#define __WEB_USB_MANAGING

#include <mutex>
#include <condition_variable>

//for rerunning any of the thread. It has meaning of return values of the threads
extern std::mutex mut_web;
extern std::condition_variable cond_web;
extern int WebServer_ToJoin;

extern std::mutex mut_usb;
extern std::condition_variable cond_usb;
extern int USBHandler_ToJoin;

#endif
