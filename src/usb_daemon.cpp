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

#include "usb_daemon.hpp"

#include "web_usb_manage.h"
#include "usb_handler.hpp"
#include "web_server.hpp"

int TUSB_Daemon::DaemonFunction()
{
	//This code runs in the second parent - WebServer
    WriteLog("%s [DAEMON] I'm baby!\n", getTime());
    
    //Run WebServer and USBHandler in threads
    web_thr=new std::thread(Web_thrFunc,1,2500);//1 connection,2500 port
    usb_thr=new std::thread(Usb_thrFunc);
    return 0;
}

int LoadConfig(const char* filename)
{
	FILE* f=fopen(filename ,"r");
	
	fclose(f);
	return 0;
}

TUSB_Daemon::~TUSB_Daemon()
{
    delete web_thr;
    delete usb_thr;
}
