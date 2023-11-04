
# Webserv should run before the execution of this script
# Set DOMAIN - keep format ( Domain:Port ) 
DOMAIN = "localhost:7700"
# There should be a /post-bin in your Root Directory to test POST Requests
# Provide Paths for CGI Testing:
CGI_TEST_SCRIPT = "/cgi-bin/test.cgi"
CGI_ENDLESS_LOOP = "/cgi-bin/endless.cgi"
CGI_PRINT_ENVIRON = "/cgi-bin/environ_c.cgi"


import subprocess
import socket
import time
import string
import random

TESTFILE = ".testcases"

RED = "\033[91m"
MAGENTA = "\033[95m"
RESET = "\033[0m"

def sendChunks(sock, *chunks):
    for chunk in chunks:
        send = hex(len(chunk))[2:].encode('utf-8') + b"\r\n" + chunk.encode('utf-8') + b"\r\n" 
        sock.send(send)
        time.sleep(0.1)
    sock.send(b"0\r\n\r\n")

def getPort():
    parts = DOMAIN.split(":")
    return int(parts[1])

def randomName(size):
    return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(size))

def printHeader(data):
    split_lines = data.splitlines()
    for line in split_lines:
        if not line:
            break
        print(line)


def testsFromTestFile():
    print(MAGENTA + "Starting with Testcases from .testfile ....\n",RESET)
    with open(TESTFILE, 'r') as file:
        lines = file.readlines()
        for i in range(0, len(lines), 2):
            description = lines[i].strip()
            command = lines[i + 1].strip()
            command = command.replace("Domain", DOMAIN)
            print(RED + "\nTest:", description)
            print("Bash Command:", command)
            print("\n++++ Return ++++", RESET)
            result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            printHeader(result.stdout)
            print(RED + "++++ END ++++", RESET)

def testChunkedRequests():
    print(MAGENTA + "\nTesting Chunked Request ..."+ RESET)
    print(RED, "\n" "GET\n", RESET)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', getPort()))
    sock.send(b"GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n")
    time.sleep(0.1)
    sendChunks(sock, "chunk 1", "chunk 2")
    data = sock.recv(4096)
    print(RED,"\n++++ Return ++++", RESET)
    printHeader(data.decode('utf-8')) 
    print(RED + "\n++++ END ++++", RESET)
    print(RED + "\nPOST into Post-bin with random Filename")
    sock.send(b"POST /post-bin/"+ randomName(5).encode('utf-8') + b" HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n")
    time.sleep(0.1)
    sendChunks(sock, "chunk 1", "chunk 2")
    data = sock.recv(4096)
    print("\n++++ Return ++++", RESET)
    printHeader(data.decode('utf-8')) 
    print(RED + "++++ END ++++", RESET)
    print(RED + "\nPOST send two chunks at once")
    sock.send(b"POST /post-bin/"+ randomName(5).encode('utf-8') + b" HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n")
    chunk1 = hex(len("chunk1"))[2:].encode('utf-8') + b"\r\n" + "chunk1".encode('utf-8') + b"\r\n"
    chunk2 = hex(len("chunk2"))[2:].encode('utf-8') + b"\r\n" + "chunk2".encode('utf-8') + b"\r\n" 
    chunk3 = hex(len("chunk3"))[2:].encode('utf-8') + b"\r\n" + "chunk3".encode('utf-8') + b"\r\n" 
    chunk4 = b"0\r\n\r\n"
    time.sleep(0.1)
    sock.send(chunk1)
    time.sleep(0.1)
    sock.send(chunk2 + chunk3)
    time.sleep(0.1)
    sock.send(chunk4)
    time.sleep(0.1)
    data = sock.recv(4096)
    print("\n++++ Return ++++", RESET)
    printHeader(data.decode('utf-8')) 
    print(RED + "++++ END ++++", RESET)
    sock.close()

def testCGI():
    print(MAGENTA + "\nTesting CGI ..."+ RESET)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', getPort()))
    
    sock.send(b"POST " + CGI_ENDLESS_LOOP.encode('utf-8') + b" HTTP/1.1\r\n\r\n")
    data = sock.recv(4096)
    print(RED + "\nCGI: Endless loop")
    print("\n++++ Return ++++", RESET)
    printHeader(data.decode('utf-8')) 
    print(RED + "\n++++ END ++++", RESET)

    sock.send(b"POST " + CGI_TEST_SCRIPT.encode('utf-8') + b" HTTP/1.1\r\n\r\n")
    data = sock.recv(4096)
    print(RED + "\nCGI: Test script")
    print("\n++++ Return ++++", RESET)
    printHeader(data.decode('utf-8')) 
    print(RED + "\n++++ END ++++", RESET)

    sock.send(b"POST " + CGI_PRINT_ENVIRON.encode('utf-8') + b" HTTP/1.1\r\n\r\n")
    data = sock.recv(4096)
    print(RED + "\nCGI: Print Enviroment")
    print("\n++++ Return ++++", RESET)
    print(data.decode('utf-8')) 
    print(RED + "\n++++ END ++++", RESET)

testsFromTestFile()
testChunkedRequests()
testCGI()
