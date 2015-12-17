import socket
s = socket.socket( socket.AF_INET , socket.SOCK_STREAM )
s.connect( ( "127.0.0.1" , 8082 ) )
s.send( 'hello' )
buf = s.recv( 64 )
print buf
