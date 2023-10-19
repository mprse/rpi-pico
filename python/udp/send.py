import random
import socket
import time

try:

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    addr = ('192.168.43.42', 9000)
	
    while True:
        client_socket.sendto(bytes('Test','utf-8'), addr)
        time.sleep(1)
except KeyboardInterrupt:
    pass