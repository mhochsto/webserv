import socket
import subprocess
import os
path = './webserv'
from subprocess import Popen, PIPE

p = subprocess.Popen(path, capture_output=False)


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 8080))
sock.send(b"GET / HTTP/1.1\r\n")
response = sock.recv(4096)
print(response.decode())
sock.close()
p.kill()
