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
    sock.send(b"GET /website/pages HTTP/1.1\r\nContent-Length=10\r\n\r\ntest123456\r\n")


p = subprocess.Popen(path, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
t = threading.Thread(target= captureServ, args=(p,))
t.start()
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
time.sleep(1.5)
sock.connect(('localhost', 7700))
testCase1(sock)

p.kill()
t.join()
sock.close()
