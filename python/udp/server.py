import random
import socket
import time 
from datetime import datetime

try:
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1) 
    server_socket.bind(('', 9000))

    while True:
        message, address = server_socket.recvfrom(50)
        data = message.decode("utf-8")
        print(data)

except KeyboardInterrupt:
    pass