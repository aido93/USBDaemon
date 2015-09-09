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

#ifndef __USB_DAEMON
#define __USB_DAEMON

#include "daemon.hpp"
#include <thread>

class TUSB_Daemon : public TDaemon{
    private:
        unsigned int connects=1;
        unsigned int port=2500;
		std::thread *web_thr,*usb_thr;
    public:
        int DaemonFunction() final;
        int LoadConfig(const char*) final;
        TUSB_Daemon(const char* ConfigName) : TDaemon(ConfigName)
        {
            //загружаем конфиг. Мы не могли этого сделать ранее, 
            //так как функция - виртуальная
            LoadConfig(ConfigName);
        };
        ~TUSB_Daemon();
};

#endif
