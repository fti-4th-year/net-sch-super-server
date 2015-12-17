#include <iostream>
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
int main( int argc , char * const * argv )
{
	std::string msg = "default";
	while( true )
	{
		char buf[ 0x1000 ];
		memset( buf , 0 , 0x1000 );
		int l = read( 0 , buf , sizeof( buf ) - 1 );
		if( buf[ 0 ] == '\n' )
		{
			return 0;
		}
		write( 1 , buf , l );
	}
	return 0;
}
