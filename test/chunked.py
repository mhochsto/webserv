import socket
import subprocess
import os
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
time.sleep(1)
sock.connect(('localhost', 7700))
sock.send(b"GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n")
time.sleep(0.1)
sock.send(b"16\r\nTest Chunk 1\r\n")
time.sleep(0.1)
sock.send(b"16\r\nTest Chunk 2\r\n")
time.sleep(0.1)
sock.send(b"0\r\n\r\n")
time.sleep(4)
sock.close()
