#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include "cfunctions.h"
#include "protocol.h"
#include "web_server.hpp"
#include "web_usb_comm.h"

//Set/Get Port
void TWebServer::SetPort(const unsigned short int &p)
{
    port=p;
}
unsigned short int TWebServer::GetPort()
{
    return port;
}
//Set/Get connections
void TWebServer::SetConnectionsCount(const unsigned short int & conn)
{
    connections=conn;
}
unsigned short int TWebServer::GetConnectionsCount()
{
    return connections;
}

int TWebServer::EstablishConnection()
{
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) 
	{
		WriteLog("%s [WEB_SERVER] Error! Couldn't create socket: %s\n", getTime(),strerror(errno));
        close(listen_sock);
		return errno;
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		WriteLog("%s [WEB_SERVER] Error! Bind Failed: %s\n", getTime(),strerror(errno));
        close(listen_sock);
		return errno;
	}
	if(listen(listen_sock, connections) < 0)
    {
        WriteLog("%s [WEB_SERVER] Error! Listen Failed: %s\n", getTime(),strerror(errno));
        close(listen_sock);
		return errno;
    }
	return 0;
}

int TWebServer::ReceiveMsg(TPacket1 & msg)
{
	ssize_t rv=0;
    accept_sock = accept(listen_sock, (struct sockaddr *) &cli_addr, &clen);
    if (accept_sock < 0) 
    {
        WriteLog("%s [WEB_SERVER] Error! Accept Failed: %s\n", getTime(),strerror(errno));
        rv=-1;
        //close(accept_sock);
        //break;
    }
    else
    {
        memset(&msg, 0, sizeof(struct TPacket1));
        rv=read(accept_sock, &msg, sizeof(struct TPacket1));
        if(rv==-1)
            WriteLog("%s [WEB_SERVER] Error! Read from socket failed: %s\n", getTime(),strerror(errno));
    }

	return rv;
}

int TWebServer::SendMsg(const std::string & msg)
{
    ssize_t rv=0;
    rv=write(accept_sock, msg.c_str(), msg.length());
    if(rv==-1)
    {
        WriteLog("%s [WEB_SERVER] Error! Write to socket failed: %s\n", getTime(),strerror(errno));
        close(accept_sock);
    }
    
    return rv;
}
TWebServer::TWebServer(const unsigned short int &conn, const unsigned short int &p)
{
    SetConnectionsCount(conn);
    SetPort(p); 
    EstablishConnection();
}
void Web_thrFunc(const unsigned short int &conn, const unsigned short int &port)
{
	WriteLog("%s [WEB_SERVER] Started\n", getTime());
    TWebServer ws(conn,port);
    WriteLog("%s [WEB_SERVER] Connection established\n", getTime());
    while(1)
    {
        //Receive Message
        {
            std::unique_lock<std::mutex> lock(m1);
            ws.ReceiveMsg(pkg);
            //signal USBHandler to proceed message
            received=true;
            cv1.notify_one();
        }
        //Send Message
        {
            std::unique_lock<std::mutex> lock(m1);
            cv1.wait(lock,[](){return ready;});
            ws.SendMsg(ans);
            ready=false;
        }
    }
    WriteLog("%s [WEB_SERVER] Finished with code: %d\n", getTime(), WebServer_ToJoin);
}
