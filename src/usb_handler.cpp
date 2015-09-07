#include "usb_handler.hpp"
#include "web_usb_comm.h"
#include <mutex>
#include <condition_variable>

TUSB_Handler::TUSB_Handler()
{
	rv=libusb_init(&ctx);
	if(rv<0) WriteLog("%s [DAEMON] Error! Failed to init libusb\n", getTime());
}

int TUSB_Handler::GetDevice(unsigned int info, libusb_device *dev, std::string & ret)
{
	char conf=0;
	char interf=0;
	char EP=0;
	
	int err=0;
	uint16_t bcdUSB=0;
	unsigned char tmp[128];
    bzero(tmp,sizeof(tmp));
	//Open the device
    libusb_device_handle *Dev;
	err = libusb_open(dev, &Dev);
	if (err < 0)
	{
		WriteLog("%s [USB_HANDLER] Failed to open device: %s\n", getTime(), strerror(errno));
		return -1;
	}
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config=new struct libusb_config_descriptor;
	rv = libusb_get_device_descriptor(dev, &desc); // look in description of current device
	if(rv<0)
	{
		WriteLog("%s [USB_HANDLER] Failed to get device descriptor!\n", getTime());
		return -1;
	}
	//Device
		if(info&GET_NUM_OF_CONFIGURATIONS)
		{
			conf=desc.bNumConfigurations;
            ret+="NC";
            ret+=std::to_string(conf);
		}
		if(info&GET_MANUFACTURER)
		{
			err = libusb_get_string_descriptor_ascii(Dev, desc.iManufacturer, (unsigned char*) tmp, sizeof(tmp));
			if (err < 0)
			{
				 WriteLog("%s [USB_HANDLER] Failed to get Manufacturer of the device: %s\n", getTime(), strerror(errno));
				 libusb_close(Dev);
                 return -1;
			}
            ret+="<MA>";
            ret+=std::string((const char*)(tmp));
		}
		if(info&GET_PRODUCT)
		{
			bzero(tmp,sizeof(tmp));
			err = libusb_get_string_descriptor_ascii(Dev, desc.iProduct, (unsigned char*) tmp, sizeof(tmp));
			if (err < 0)
			{
				WriteLog("%s [DAEMON] Failed to get Product of the device: %s\n", getTime(), strerror(errno));
				libusb_close(Dev);
                return -1;
			}
			ret+="<PR>";
            ret+=std::string((const char*)(tmp));
		}
		if(info&GET_VENDOR_ID)
		{
            ret+="VI";
            ret+=std::to_string(desc.idVendor);
		}
		if(info&GET_PRODUCT_ID)
		{
			ret+="PI";
            ret+=std::to_string(desc.idProduct);
		}
		if(info&GET_DEVICE_TYPE)
		{
            ret+="DC";
            ret+=std::to_string(desc.bDeviceClass);
		}
		if(info&GET_USB_TYPE)
		{
			bcdUSB=desc.bcdUSB;
            ret+="UT";
            ret+=(std::to_string((bcdUSB&0xFF00)>>8)+std::to_string((bcdUSB&0x00FF)>>4));
		}
	//Configuration
		if(info&GET_CONFIGURATION)
		{
			conf=(info&0x00700000)>>20;
            rv=libusb_get_config_descriptor(dev,conf-1,&config);		
            if(rv<0)
            {
                WriteLog("%s [USB_HANDLER] Failed to get configuration descriptor!\n", getTime());
                return -1;
            }
			if(info&GET_RETURN_LENGTH)
			{
                ret+="RT";
                ret+=std::to_string(config->wTotalLength);
			}
			if(info&GET_POWER_TYPE)
			{
                ret+="PT";
                ret+=std::to_string(config->bmAttributes);
			}
			if(info&GET_MAX_POWER)
			{
                ret+="MP";
                ret+=std::to_string(config->MaxPower);
			}
			if(info&GET_NUM_INTERFACES)
			{
                ret+="NI";
                ret+=std::to_string(config->bNumInterfaces);
				//Interface
				if(info&GET_INTERFACE)
				{
					interf=(info&0x00007000)>>12;
                    const struct libusb_interface_descriptor *interface=\
                        &config->interface->altsetting[interf-1];
					if(info&GET_CLASSES)
					{
						ret+="CS";
                        ret+=std::to_string(interface->bInterfaceClass)+\
                        ":"+std::to_string(interface->bInterfaceSubClass);
					}
					if(info&GET_PROTOCOL)
					{
						ret+="PL";
                        ret+=std::to_string(interface->bInterfaceProtocol);
					}
                    if(info&GET_ALTERNATE_SETTING)
					{
                        ret+="AS";
                        ret+=std::to_string(interface->bAlternateSetting);
                    }
					if(info&GET_NUMBER_OF_EPS)
					{
						ret+="NE";
                        ret+=std::to_string(interface->bNumEndpoints);						
						//EndPoints
						if(info&GET_EP)
						{
							EP=(info&0x00000070)>>4;
                            const struct libusb_endpoint_descriptor \
                                *endpoint=&interface->endpoint[EP-1];
							if(info&GET_EP_ADDRESS)
							{
								ret+="EA";
                                ret+=std::to_string(endpoint->bEndpointAddress);
							}
							if(info&GET_MAX_PACKET_SIZE)
							{
								ret+="PS";
                                ret+=std::to_string(endpoint->wMaxPacketSize);
							}
							if(info&GET_EP_TYPE)
							{
								ret+="ET";
                                ret+=std::to_string(endpoint->bmAttributes);
							}
							if(info&GET_INTERVAL)
							{
								ret+="EI";
                                ret+=std::to_string(endpoint->bInterval);
							}
						}
					}
				}
			}
            libusb_free_config_descriptor(config);
		}
		//ret=(TAnswer*)malloc(bar->length);
		//memcpy(ret,bar,bar->length);
		libusb_close(Dev);
		return 0;
}

char TUSB_Handler::Handler(std::string & ret, const struct TPacket1 & packet)
{
    pkg=packet;
    unsigned int counter=pkg.GetUSBLength;
    short count=0;
    int i=0;
    int rv = libusb_get_device_list( ctx, &devs); //get the list of devices
    if(rv<0)
    {
        WriteLog("%s [USB_HANDLER] There are none USB devices on bus\n", getTime());
    }
    else
    {//all is ok
        if(pkg.info.info!=0)
        {
            if(pkg.info.info&LIST_ALL)
            {//LIST_ALL
                WriteLog("%s [USB_HANDLER] ListAll:\n", getTime());
                while( (dev=devs[i++]) )
                {
                    GetDevice(pkg.info.info,dev,ret);
                    ret+='\n';//One device - one line
                    count++;
                }
            }
            else
            while(counter>0)
            {//LIST_ONE
                WriteLog("%s [USB_HANDLER] ListOne:\n", getTime());
                GetDevice(pkg.info.info,devs[pkg.info.DeviceNumber-1],ret);
                ret+='\n';//One device - one line
                counter--;
                count++;
            }
        }
    }
    return count;
}

void Usb_thrFunc()
{
    WriteLog("%s [USB_HANDLER] Started\n", getTime());
    TUSB_Handler usb_h;
    char count=0;
    while(1)
    {
        received = false;
        //waiting message
        {
            std::unique_lock<std::mutex> lk(m1);
            cv1.wait(lk, []{return received;});
            //forget previous answer and set first 8 bytes:
            //URS10 - header=Universal Robotics System v. 1.0;
            //1,2 - length of the message in bytes;
            //3 - count of devices;
            ans="URS10000\n";
            count=usb_h.Handler(ans,pkg);
            unsigned short int l=ans.length();
            ans.replace(5,2,std::string((const char*)&l,2));
            ans.replace(6,1,std::string(1,count));
            WriteLog("%s [USB_HANDLER] Message: \n%s\n", getTime(),ans.c_str());
            //sending message to thread
            ready=true;
            cv1.notify_one();
        }
    }
}
