import socket
import struct
import subprocess
from ble_stuff import boink
import csv

HOST = '127.0.0.1'
PORT = 12345

subprocess.Popen(["./main"])

class ADS_Data:
    def __init__(self, id=0, ra=0, ll=0, la=0, v1=0):
        self.id = id
        self.ra = ra
        self.ll = ll
        self.la = la
        self.v1 = v1

class BHI_Data:
    def __init__(self, id=0, imuNum=0, x=0, y=0, z=0):
        self.id = id
        self.imuNum = imuNum
        self.x = x
        self.y = y
        self.z = z

class Sensors:
    def __init__(self):
        self.sensor_1 = BHI_Data()
        self.sensor_2 = BHI_Data()
        self.sensor_3 = BHI_Data()
        self.sensor_4 = BHI_Data()
        self.sensor_5 = BHI_Data()

BHI_data_Buffer = [Sensors() for _ in range(300)]
ADS_data_Buffer = [ADS_Data() for _ in range(3000)]

ADS_buffer = "Iiiii" * 3000
BHI_buffer = ("Iiiii" * 5) * 300

def receive_data(socket):
    data1 = socket.recv(struct.calcsize("U"))
    if(data1 == b'E'):
         # Receive binary data from the socket
        ecg_data = socket.recv(struct.calcsize(ADS_buffer))
        # Unpack binary data into DataObject
        ADS_data_Buffer = struct.unpack(ADS_buffer,ecg_data)
        return ADS_data_Buffer
    elif(data1 == b'A'):
        acc_data = socket.recv(struct.calcsize(BHI_buffer))
        BHI_data_Buffer = struct.unpack(ADS_buffer,acc_data)
        return BHI_data_Buffer
    

STATUS = int(input("press 1 - Simulation data, press 2 - Actual data: "))

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.send(struct.pack("I", STATUS))

    # with open('received_data.csv', 'w', newline='') as csvfile:
    #     fieldnames = ['id', 'ra', 'll', 'la', 'v1', 'as1', 'as2', 'as3', 'as4']
    #     writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    #     writer.writeheader()

    #     while True:
    #         data = receive_data(s)
    #         if not data:
    #             break
    #         # Process the received data in real-time
    #         # print(data.__dict__)

    #         # Write the data to the CSV file
    #         writer.writerow({
    #             'id': data.id,
    #             'ra': data.ra,
    #             'll': data.ll,
    #             'la': data.la,
    #             'v1': data.v1,
    #             'as1': data.as1,
    #             'as2': data.as2,
    #             'as3': data.as3,
    #             'as4': data.as4
    #         })

