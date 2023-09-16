import socket
import subprocess
import os
import time
import threading
from subprocess import Popen, PIPE

path = './webserv'

def captureServ(p):
    out = p.communicate()
    print('\033[94m')
    print(out)

p = subprocess.Popen(path, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
t = threading.Thread(target= captureServ, args=(p,))
t.start()
time.sleep(0.1)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 7700))
sock.send(b"GET /website/pages HTTP/1.1\r\n")
response = sock.recv(4096)
print("\x1b[31m" + response.decode())
p.kill()
t.join()
sock.close()
