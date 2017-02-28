# coding: utf-8
import sys
import socket
import numpy as np
import matplotlib.pyplot as plt

def main():
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.bind(('', 5151))


	while True:
		sock.listen(5)
		client, address = sock.accept()
		print "{} connected".format( address )

		response = client.recv(1000000)
		
		if response != "":
			print response
			print len(response)
			pic = string_to_matrix(response)
			print pic
			plt.imshow(pic)
			plt.show()

	print "Close"
	client.close()
	stock.close()
		
		
def string_to_matrix(str):
	size,data = str.split("-")
	row,col = size.split("x")
	data = data.split(',')
	data.pop(-1) # Remove last empty element
	data = np.array(data,dtype=int)
	data = data.reshape((int(row),int(col)))
	return data
	
if __name__ == "__main__":
	try:
		main()
	except KeyboardInterrupt, e:
		print(e)
		sys.exit(0)