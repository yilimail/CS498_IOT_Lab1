import socket
import tkinter as tk
import json

UDP_IP = "169.254.190.122"  #The ip address that u are listening to
UDP_PORT = 54322   #Local port number, as long as the port is free and could be used by user.

sock = socket.socket(socket.AF_INET, # Internet
                      socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
data, addr = sock.recvfrom(1024)
data = json.loads(data)
print(data)
#root = tk.Tk()
#label = tk.Label(root, str(data), width=50, height=25)
#label.pack()
#T.insert(tk.END, next(str(data)))
#def refresh():
	#try:
	  #label['text'] = next(data)
	  #root.after(1000,refresh)
	#except StopIteration:
	  #root.destroy()
#root.after(1000,refresh)
#tk.mainloop()

# while True:
#  data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
#  print ("received message:", data)

