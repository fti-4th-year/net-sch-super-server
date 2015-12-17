#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
int main( int argc , char * const * argv )
{
	int sock_id = socket( AF_INET , SOCK_STREAM , 0 );
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	server.sin_family = AF_INET;
	server.sin_port = htons( atoi( argv[ 1 ] ) );
	connect( sock_id , ( struct sockaddr * )&server , sizeof( server ) );
	while( true )
	{
		char buf[ 0x1000 ];
		memset( buf , 0 , 0x1000 );
		int l = read( 0 , buf , sizeof( buf ) - 1 );
		if( l == 0 || buf[ 0 ] == '\n' )
		{
			goto exit;
		}
		buf[ --l ] = 0;
		std::cout << "sending " << buf << "...\n";
		buf[ l ] = '\0';
		send( sock_id , buf , l + 1 , 0 );
		memset( buf , 0 , 0x1000 );
		l = recv( sock_id , buf , 0x1000 , 0 );
		if( l == 0 )
		{
			goto exit;
		}
		std::cout << "recieved:" << buf << "\n";
	}
exit:
	close( sock_id );
	return 0;
}
