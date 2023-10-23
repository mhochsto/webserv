import socket
testCases = "testcases.txt"



def test_line(sock, line):
    sock.send(line)


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 7700))


with open(testCases, "rb") as file:
    test_line(sock, file.readline())
