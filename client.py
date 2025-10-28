import socket

HOST = '127.0.0.1'
PORT = 8080

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((HOST, PORT))

isFirst = True

while True:
    data = client.recv(1024)
    print(data.decode(), end='')

    if not isFirst:
        data = client.recv(1024)
        print(data.decode(), end='')

    isFirst = False

    # Get full line input
    msg = input()
    client.sendall((msg + "\n").encode())
