#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
int main( int argc , char * const * argv )
{
	int sock_id = socket( AF_INET , SOCK_STREAM , 0 );
	sockaddr_in _server;
	_server.sin_family = AF_INET;
	_server.sin_addr.s_addr = INADDR_ANY;
	_server.sin_port = htons( atoi( argv[ 1 ] ) );
	bind( sock_id , ( sockaddr * )&_server , sizeof( _server ) );
	listen( sock_id , 10 );
	int client_sock;
	sockaddr_in client;
	int c = sizeof( client );
	client_sock = accept( sock_id , ( sockaddr * )&client , ( socklen_t* )&c );
	std::string msg;
	char buf[ 0x1000 ];
	std::cout << "connected with fd:" << client_sock << "\n";
	recv( client_sock , buf , 0x1000 , 0 );
	std::cout << "recieved:" << buf << "\n";
	std::cin >> msg;
	send( client_sock , msg.c_str() , msg.size() , 0 );
	close( client_sock );
	close( sock_id );
	return 0;
}
