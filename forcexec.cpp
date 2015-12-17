#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
int main( int argc , char * const * argv )
{
	#define PARENT_WRITE_PIPE  0
	#define PARENT_READ_PIPE   1

	#define READ_FD  0
	#define WRITE_FD 1

	#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
	#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )

	#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
	#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )
	int outfd[2];
	int infd[2];
	int pipes[2][2];

	pipe(pipes[PARENT_READ_PIPE]);
	pipe(pipes[PARENT_WRITE_PIPE]);

	if( !fork() )
	{
		char *argv[]=
		{	"printer", "-q", 0};
		dup2(CHILD_READ_FD, STDIN_FILENO);
		dup2(CHILD_WRITE_FD, STDOUT_FILENO);
		//std::cout << "hello" << std::endl;
		close(CHILD_READ_FD);
		close(CHILD_WRITE_FD);
		close(PARENT_READ_FD);
		close(PARENT_WRITE_FD);

		execv( argv[0] , argv );
	} else
	{
		char buffer[100];
		int count;

		close(CHILD_READ_FD);
		close(CHILD_WRITE_FD);
		write(PARENT_WRITE_FD, "HELLO\n", 6);
		
		count = read( PARENT_READ_FD , buffer , sizeof( buffer ) -1 );
		buffer[count] = 0;
		std::cout << "from parent:" << buffer;
	}
}
