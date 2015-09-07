#ifndef __USB_WEBSERVER
#define __USB_WEBSERVER

#include "protocol.h"
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

void Web_thrFunc(const unsigned short int &conn, const unsigned short int &port);

class TWebServer
{
    private:
        int listen_sock;
        int accept_sock;
        struct sockaddr_in serv_addr;
        struct sockaddr_in cli_addr;
        socklen_t clen;
        
        int EstablishConnection();
        unsigned short int port;
        unsigned short int connections;
    public:
        TWebServer(const unsigned short int &conn, const unsigned short int &p);
        ~TWebServer(){};
        void SetPort(const unsigned short int &port);
        unsigned short int GetPort();
        void SetConnectionsCount(const unsigned short int &conn);
        unsigned short int GetConnectionsCount();
        int ReceiveMsg(TPacket1 &);
        int SendMsg(const std::string &);
};
#endif
