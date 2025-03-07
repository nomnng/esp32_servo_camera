import socket
from PIL import Image
from io import BytesIO

s = socket.socket()
s.connect(("192.168.0.102", 12345))

ota_data = open("build/servo_camera.bin", "rb").read()
s.send(int(101).to_bytes())
s.send(int(len(ota_data)).to_bytes(4, "little"))
s.sendall(ota_data)

print("Waiting until the upload is finished!")
s.recv(1024)
input("Finished!")
