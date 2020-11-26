from socket import socket, AF_INET, SOCK_DGRAM

IP = "127.0.0.1"
PORT = 8000

sock = socket(AF_INET, SOCK_DGRAM)   # создание UDP-сокета
sock.bind((IP, PORT))                    # нужно анонсировать порт  

# Соединение не требуется
MAX_SIZE = 4
while True:
    data, addr = sock.recvfrom(MAX_SIZE) # получить сообщение
    print("Got", data)
    sock.sendto(data, addr)
