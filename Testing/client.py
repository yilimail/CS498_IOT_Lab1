import socket

UDP_IP_ADDRESS = "169.254.91.120"
UDP_PORT_NO = 54322
Message = b"Hello, Server"
clientSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while(True):
	clientSock.sendto(Message, (UDP_IP_ADDRESS, UDP_PORT_NO))
