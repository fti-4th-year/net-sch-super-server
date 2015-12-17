import sys
sys.stdout.write( "python started" )
#sys.stdout.flush()
msg = ""
while True:
	try:
		msg += sys.stdin.read(1)
		if msg.endswith( '\0' ):
			sys.stdout.write( "message from python:%s" % msg )
			msg = ""
		#sys.stdout.flush()
	except:
		exit()
