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

def testCase1(sock):
    sock.send(b"GET /website/pages HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n")
    time.sleep(0.1)
    sock.send(b" E\r\nthis is a test\r\n")
    time.sleep(0.1)
    sock.send(b"12\r\nthis is a new test\r\n")
    time.sleep(0.1)
    sock.send(b"0\r\n\r\n")

#p = subprocess.Popen(path, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
#t = threading.Thread(target= captureServ, args=(p,))
#t.start()
#time.sleep(0.1)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 7700))
testCase1(sock)




#response = sock.recv(4096)
#print("\x1b[31m" + response.decode())
#p.kill()
#t.join()
sock.close()
