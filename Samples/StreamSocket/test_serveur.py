# import socket

# sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# server_adress = ('localhost', 12800)
# sock.bind(server_adress)
# sock.listen(5)

# print("Le serveur ecoute sur le port:")

# connexion_avec_client, infos_connexion = sock.accept()

import socket

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.bind(('', 15555))

while True:
        socket.listen(5)
        client, address = socket.accept()
        print "{} connected".format( address )

        response = client.recv(255)
        if response != "":
                print response

print "Close"
client.close()
stock.close()