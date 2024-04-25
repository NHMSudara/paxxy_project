import socket
import struct
import subprocess
import csv

HOST = '127.0.0.1'
PORT = 12345

subprocess.Popen(["./main"])

# Define the DataObject structure in Python
class DataObject:
    def __init__(self, id, ra, ll, la, v1, as1, as2, as3, as4):
        self.id = id
        self.ra = ra
        self.ll = ll
        self.la = la
        self.v1 = v1
        self.as1 = as1
        self.as2 = as2
        self.as3 = as3
        self.as4 = as4

def receive_data(socket):
    # Receive binary data from the socket
    data = socket.recv(struct.calcsize("Iffffiiii"))

    # Unpack binary data into DataObject
    id, ra, ll, la, v1, as1, as2, as3, as4 = struct.unpack("Iffffiiii", data)

    return DataObject(id, ra, ll, la, v1, as1, as2, as3, as4)

STATUS = int(input("press 1 - Simulation data, press 2 - Actual data: "))

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.send(struct.pack("I", STATUS))

    with open('received_data.csv', 'w', newline='') as csvfile:
        fieldnames = ['id', 'ra', 'll', 'la', 'v1', 'as1', 'as2', 'as3', 'as4']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        while True:
            data = receive_data(s)
            if not data:
                break
            # Process the received data in real-time
            # print(data.__dict__)

            # Write the data to the CSV file
            writer.writerow({
                'id': data.id,
                'ra': data.ra,
                'll': data.ll,
                'la': data.la,
                'v1': data.v1,
                'as1': data.as1,
                'as2': data.as2,
                'as3': data.as3,
                'as4': data.as4
            })

