#ifndef __USB_DAEMON_HANDLER
#define __USB_DAEMON_HANDLER

#include "protocol.h"
#include "cfunctions.h"
#include <libusb-1.0/libusb.h>
#include <string>

void Usb_thrFunc();

class TUSB_Handler
{
    private:
        int rv=1;
        TPacket1 pkg;
        //static struct libusb_device_descriptor desc; //description of device structure
		//static libusb_device_handle *dev_handle;   //a device handle
		libusb_context       *ctx;           //a libusb session
		libusb_device        *dev;          //pointer to device
		libusb_device        **devs;        //pointer to pointer of device
        int GetDevice(unsigned int, libusb_device *, std::string &);
    public:
        char Handler(std::string &, const struct TPacket1 &);
        TUSB_Handler();
        ~TUSB_Handler(){};
};

#endif
