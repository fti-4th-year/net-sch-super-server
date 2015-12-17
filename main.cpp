/*
 * main.cpp
 *
 *  Created on: Dec 9, 2015
 *      Author: anton
 */
#include <tcp/Socket.h>
#include <tcp/SocketHandler.h>
#include <tcp/HTTPRequestFactory.h>
#include <list>
#include <tcp/SocketFactory.h>
#include <tcp/SocketListener.h>
#include <sys/signal.h>
#include <tcp/Socks4Manager.h>
#include <iostream>
#include <string>
#include <regex>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
std::vector< std::string > &split( const std::string &s , char delim , std::vector< std::string > &elems )
{
	std::stringstream ss( s );
	std::string item;
	while( std::getline( ss , item , delim ) )
	{
		if( item.length( ) > 0 )
		{
			elems.push_back( item );
		}
	}
	return elems;
}
std::vector< std::string > split( const std::string &s , char delim )
{
	std::vector< std::string > elems;
	split( s , delim , elems );
	return elems;
}
Waiter waiter;
#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1
#define READ_FD  0
#define WRITE_FD 1
#define PARENT_READ_FD  pipes[PARENT_READ_PIPE][READ_FD]
#define PARENT_WRITE_FD pipes[PARENT_WRITE_PIPE][WRITE_FD]
#define CHILD_READ_FD   pipes[PARENT_WRITE_PIPE][READ_FD]
#define CHILD_WRITE_FD  pipes[PARENT_READ_PIPE][WRITE_FD]
class ProcessPipe : public Sleepy
{
public:
	int getId( ) const override
	{
		return PARENT_READ_FD;
	}
public:
	bool valid = false;
	int pipes[ 2 ][ 2 ];
	pid_t pid;
	std::string _cmd;
	void create( SocketHandler sh , std::string cmd )
	{
		_cmd = cmd;
		/*pipe( pipes[ PARENT_READ_PIPE ] );
		pipe( pipes[ PARENT_WRITE_PIPE ] );*/
		std::vector< std::string > cmds = split( cmd , ' ' );
		struct Param
		{
			char param[ 100 ] = { 0 };
		};
		std::unique_ptr< Param[] > params( new Param[ cmds.size() ]);
		std::unique_ptr< char*[] > argv( new char*[ cmds.size() + 1 ]);
		for( int i = 0; i < cmds.size();i++)
		{
			memcpy( &(params[ i ]) , cmds[ i ].c_str( ) , cmds[ i ].size( ) );
			argv[ i ] = params[ i ].param;
		}
		
		
		argv[ cmd.size() ] = 0;
		if( !( pid = fork( ) ) )
		{
			//char * const argv[] = { path , param , 0 };
			dup2( sh.getSocket()->getId() , STDIN_FILENO );
			dup2( sh.getSocket()->getId() , STDOUT_FILENO );
			//dup2( CHILD_READ_FD , STDIN_FILENO );
			//dup2( CHILD_WRITE_FD , STDOUT_FILENO );

			/*close( CHILD_READ_FD );
			close( CHILD_WRITE_FD );
			close( PARENT_READ_FD );
			close( PARENT_WRITE_FD );*/
			if( execv( cmds[ 0 ].c_str( ) , argv.get() ) )
			{
				std::cout << "couldn't start command:" << cmd << "\n";
			}
		} else
		{
			/*close( CHILD_READ_FD );
			close( CHILD_WRITE_FD );*/
			std::cout << _cmd << " with pid:" << pid << " started\n";

		}
		valid = true;
	}
	void release( )
	{
		close( PARENT_READ_FD );
		close( PARENT_WRITE_FD );
		kill( pid , SIGKILL );
		std::cout << _cmd << " with pid:" << pid << " ended\n";
		//waitpid( pp->pid , &returnStatus , 0 );
		valid = false;
	}
};
//std::list< shared( ProcessPipe ) > process_pipes;
void setUpHttpListener( ushort const port )
{
	SocketFactory http_listener;
tryagain:
	try
	{
		std::cout << "trying to bind socket http listener...\n";
		http_listener = std::move( SocketFactory( SocketListener( port ) ) );
	} catch( std::exception const &e )
	{
		sleep( 1 );
		std::cout << "fail\n";
		goto tryagain;
	}
	std::string const portstr = std::to_string( port );
	std::cout << "success\n";
	waiter.pushSleepy({ http_listener.getSocketListener( ) , [ portstr , http_listener ]( )
		{
			try
			{
				SocketHandler in( http_listener.get( ) );
				Msg request;
				in >> request;

				std::string str = request.getString( );
				Msg answer;
				std::smatch match;
				if( std::regex_search( str , match , std::regex( " .*HTTP.*" ) ) )
				{
					std::string content_type = "text/html";
					{
						std::regex re( "GET (.*)" );

						if( std::regex_search( str , match , re ) )
						{
							std::string str = match[0];
							std::string cmd = std::regex_replace( str , std::regex( " |GET|HTTP/1.1|/" ) , "" );
							cmd = std::regex_replace( cmd , std::regex( "%20" ) , " " );
							std::vector< std::string > elems;
							std::vector< std::string > smds = split( cmd , '$' );

							/*if( cmd == "help" )
							 {
							 answer = "there is no help.";
							 } else if( cmd == "info" )
							 {
							 answer = "parser is working.";
							 } else
							 {
							 answer = "error page";
							 }*/
							for( std::string cmd : smds )
							{
								std::stringstream ss;
								struct stat sb;
								std::cout << cmd + "\n";
								FILE *fp;
								unsigned char buf[ 0x10000 ];
								fp = fopen( cmd.c_str( ) , "rb" );
								if( fp != NULL )
								{
									std::smatch match;
									if( std::regex_search( cmd , match , std::regex( "(.*)png" ) ) )
									{
										content_type = "image/png";
									} else if( std::regex_search( cmd , match , std::regex( "(.*)jpg" ) ) )
									{
										content_type = "image/jpg";
									}
									fseek( fp , 0L , SEEK_END );
									unsigned long size = ftell( fp );
									rewind( fp );
									std::cout << "file size: " << size << "\n";
									int integ = 0;
									int l = 0;

									while( ( l = read( fp ->_fileno , buf , 0x10000 ) ) > 0 )
									{
										integ += l;
										answer.add( buf , l );
									}
									std::cout << "file read: " << answer.size( ) << " " << integ << "\n";
									fclose( fp );
								} else
								{
									fp = popen( cmd.c_str( ) , "r" );
									char buf[ 0x10000 ];
									if( fp != NULL )
									{
										while( fgets( buf , 0x1000 , fp ) != NULL )
										{
											ss << buf;
										}
										pclose( fp );
									}
								}
								answer.add( ( byte* ) ss.str( ).c_str( ) , ss.str( ).size( ) );
							}
						}
					}
					if( answer.size( ) != 0 )
					{
						Msg final_answer;
						final_answer << "HTTP/1.1 200 OK\n";
						final_answer << "Content-length: ";
						final_answer << std::string( std::to_string( answer.size( ) ) );
						final_answer << "\n";
						final_answer << "Content-Type: ";
						final_answer << content_type;
						final_answer << "\n\n";
						final_answer << answer;
						in << final_answer;
					} else
					{
						Msg final_answer;
						final_answer << "HTTP/1.1 200 OK\n";
						final_answer << "Content-Type: text/html\n\n";
						final_answer << "<html><head></head><body><div>simple page</div></body></html>";
						in << final_answer;
					}
				}
			} catch( std::exception const &e )
			{
				std::cerr << "http catcher:" << e.what( ) << "\n";
			}
		} , Waiter::READ } );
}
void setUpCommand( ushort const port , std::string cmd )
{
	SocketFactory cmd_listener;
	ushort i = 0;
tryagain:
	try
	{
		std::cout << "trying to bind socket " << port + i << "->" << cmd << "...\n";
		cmd_listener = std::move( SocketFactory( SocketListener( port + i ) ) );
	} catch( std::exception const &e )
	{
		sleep( 1 );
		i++;
		std::cout << "fail\n";
		goto tryagain;
	}
	std::string const portstr = std::to_string( port );
	std::cout << "success\n";
	waiter.pushSleepy({ cmd_listener.getSocketListener( ) , [ cmd , portstr , cmd_listener ]( )
		{
			try
			{
				SocketHandler in( cmd_listener.get( ) );
				shared( ProcessPipe ) pp( new ProcessPipe( ) );
				pp->create( in , cmd );
				/*auto release = [ pp , in ]( )
				{
					pp->release( );
					waiter.removeSleepy( pp );
					waiter.removeSleepy( in.getSocket( ) );
					int returnStatus;
				};
				waiter.pushSleepy({ in.getSocket( ) , [ release , pp , in , cmd ]( )
					{
						try
						{
							if( !pp->valid )
							{
								return;
							}
							Msg req;
							in >> req;
							if( req.size( ) == 0 )
							{
								release( );
							}
							auto strreq = req.getString( );
							std::cout << "client:" << strreq << "\n";
							strreq += "\0\n";
							write( pp->PARENT_WRITE_FD , strreq.c_str( ) , strreq.size( ) );
						} catch( std::exception const &e )
						{
							release( );
							std::cerr << "cmd:" << cmd << " ProcessPipe socket catcher:" << e.what( ) << "\n";
						}
					} , Waiter::READ } );
				waiter.pushSleepy({ pp , [ release , pp , in , cmd ]( )
					{
						try
						{
							if( !pp->valid )
							{
								return;
							}
							char buffer[ 0x1000 ];
							int count;
							count = read( pp->PARENT_READ_FD , buffer , sizeof( buffer ) - 1 );
							if( count == 0 )
							{
								release( );
								return;
							}
							buffer[ count ] = 0;
							std::stringstream ss;
							ss << buffer;
							Msg final_answer;
							final_answer << ss.str( );
							std::cout << "process:" << ss.str( ) << "\n";
							in << final_answer;
						} catch( std::exception const &e )
						{
							release( );
							std::cerr << "cmd:" << cmd << " ProcessPipe socket catcher:" << e.what( ) << "\n";
						}
					} , Waiter::READ } );*/
			} catch( std::exception const &e )
			{
				std::cerr << "cmd:" << cmd << " main catcher:" << e.what( ) << "\n";
			}
		} , Waiter::READ } );
}
int main( int argc , char * const * argv )
{
	ushort const port = std::atoi( argv[ 1 ] );
	//setUpCommand( 8088 , "Debug/printer" );
	setUpHttpListener( port );
	std::ifstream config( "config.txt" );
	std::string ports , cmds;
	std::string line;
	while( std::getline( config , line ) )
	{
		std::istringstream iss( line );
		iss >> ports;
		cmds = std::string( iss.str( ).c_str( ) + ports.size( ) + 1 );
		/*if( !( iss >> ports >> cmds ) )
		 {
		 break;
		 }*/
		setUpCommand( std::atoi( ports.c_str( ) ) , cmds );
	}
	config.close( );
	int i = 0;
	bool working = true;
	while( working )
	{
		try
		{
			waiter.wait( 100 );
		} catch( std::exception const &e )
		{
			std::cerr << "waiter catcher:" << e.what( ) << "\n";
			working = false;
		}
	}
	return 0;
}

