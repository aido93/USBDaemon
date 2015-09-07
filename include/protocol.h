/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
/**
 * This file describes protocol of transferring data
 * between daemon and client
 * 
 * */
#ifndef __WEBUSB_PROTOCOL
#define __WEBUSB_PROTOCOL

#include <errno.h>
#include <stdint.h>
//need for useful structures and enums
//#include "libusb.h"

#define MaxTaskNameLength 15
//What do end user may need????
///For more information see: http://www.beyondlogic.org/usbnutshell/usb5.shtml

/**
 * Five bytes:
 * 1 - Device Properties
 * 2 - Configuration Properties
 * 3 - Interface Properties
 * 4 - Endpoint Properties
 * 5 - Number of the device in list. Needed if ListOne is set.
 * */

///First byte - getinfo. if 0x80000000 - count of devices. 
///if 0x00000000 and no configuration - no return
///First: Get Info about USB-devices in the system.
//Get Info about only one Device
#define LIST_ONE                     0x00000000
//List of All USB Devices in the system
#define LIST_ALL                     0x80000000

///Properties of entire device (for bit or)

//Get Number of configurations
#define GET_NUM_OF_CONFIGURATIONS      0x40000000
//Get Device Manufacturer
#define GET_MANUFACTURER             0x20000000
//Get Device Product
#define GET_PRODUCT                  0x10000000
//Get Vendor ID
#define GET_VENDOR_ID                 0x08000000
//Get Product ID
#define GET_PRODUCT_ID                0x04000000
//Get Device Type
#define GET_DEVICE_TYPE               0x02000000
//Get Type of protocol (USB 1.0/2.0/3.0)
#define GET_USB_TYPE                  0x01000000

///Second Byte - select concrete Configuration. (CANNOT USE without previous section)
///Max Count of configurations = 8
//Get this Configuration
#define GET_CONFIGURATION            0x00800000
//Next 3 bits for number of the configuration

//Get Total length of returned bytes
#define GET_RETURN_LENGTH             0x00080000
//Get bmAttributes. Power type=SelPower, RemotePower
#define GET_POWER_TYPE                0x00040000
//Get Max Power. step=2mA
#define GET_MAX_POWER                 0x00020000
//GetCount of Interfaces in configuration
#define GET_NUM_INTERFACES            0x00010000
/*This is unneeded for end-user because of end-user have Number of Config
 * 
///Get index of Configuration in human readable form
///#define GetIConfig                  0x0800
///Get Value to set this configuration
///#define GetConfigurationValue       0x0400
* */

///Third Byte - Select Interface of the Configuration (CANNOT USE without previous section)
//Get this Interface
#define GET_INTERFACE                0x00008000
//Next 3 bits for number of the interface. Max=8 interfaces
//Get Class, SubClass, Protocol in form Class:Subclass
#define GET_CLASSES                  0x00000800
//Get Protocol of the interface
#define GET_PROTOCOL                 0x00000400
//Get Number of EndPoints
#define GET_NUMBER_OF_EPS              0x00000200
//Get Alternate Setting
#define GET_ALTERNATE_SETTING          0x00000100

///Fourth Byte - Select EndPoint of the interface (CANNOT USE without previous section)
//Get this EndPoint
#define GET_EP                       0x00000080
//Next 3 bits for number of the interface. Max=8 endpoints
//Get address of the EP
#define GET_EP_ADDRESS                0x00000008
//Get Max Packet Size
#define GET_MAX_PACKET_SIZE            0x00000004
//Get Type (Bulk/Control/Isochronous/Interrupt and so on)
#define GET_EP_TYPE                   0x00000002
//Get Interval for Isochronous Mode
#define GET_INTERVAL                 0x00000001

#pragma pack(push, 1)
struct TGetUSBInfo{
    unsigned int info;
    char DeviceNumber;//>=0
};
#pragma pack(pop)

///Now go to commands that we can send to Daemon for operations with Device

///Always return value, except Flag Silent is set
//Get current task name
#define GetCurrentTask              0x01
//Get task status: stopped, run, error (self-debug), warning
#define GetTaskStatus               0x02
//Get list of tasks (List of tasks in Tasks Directory)
#define GetTasksList                0x03

//Start Task
#define StartTask                   0x04
//Load Task to daemon (required if we must have plan B and no time to load)
#define LoadTask                    0x05
//Unload Task from daemon
#define UnloadTask                  0x06
//Upload task to daemon's machine
#define UploadTask                  0x07
//Download task from daemon's machine (if we want to know, what it does)
#define DownloadTask                0x08
//Delete Task from Daemon's machine
#define DeleteTask                  0x09

//struct for start/load/delete task
#pragma pack(push, 1)
struct TTask
{
    char TaskName[MaxTaskNameLength];
    char signal;
};
#pragma pack(pop)

//this signals can be send in TTask without TaskName
//Restart Task
#define RestartTask                 0x0A
//Stop Task
#define StopTask                    0x0B

///Types of return values after commands
//No return. fully invisible. we don't know what happens with daemon
#define Silent                      0x0C
//Full. Required if we want full control of daemon
#define Full                        0x0D
//Only 'Get' Requests will return info
#define OnlyGet                     0x0E

#pragma pack(push, 1)
struct TPacket1
{
        unsigned short int PacketLength;
        char GetUSBLength;//minimum is one. if doesn't need - info=NULL
        struct TGetUSBInfo info;
        
        char GetTask;//if doesn't need - GetTask=NULL
        struct TTask task;//if doesn't need - task=NULL
        char SetConf;//Silent/Full/OnlyGet/NULL
};
#pragma pack(pop)

#endif
