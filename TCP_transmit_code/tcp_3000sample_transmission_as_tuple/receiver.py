# Data receiving from c file
import socket
import struct
import queue
import numpy as np
import subprocess


HOST = '127.0.0.1'
PORT = 6789

subprocess.Popen(["./main"])

#-------------------------------------------------------------------------#
# 6s ECG buffer 
class ECG:
    def __init__(self, ra, ll, la, v1):
        self.ll = ra
        self.la = ll
        self.ra = la
        self.v1 = v1

ecg_Data = ECG(0, 0, 0, 0) 
ecg_queue = [queue.Queue() for _ in range(2)]
ecg_qcount = 0
ecg_batch = 0

#-------------------------------------------------------------------------#
# 6s ACC buffer
class ACC:
    def __init__(self, imu_num=0, x=0, y=0, z=0):
        self.imu_num = imu_num
        self.x = x
        self.y = y
        self.z = z

class Sensors:
    def __init__(self):
        self.BHI = {i: ACC() for i in range(1, 6)}

acc_Data = Sensors() 
acc_queue = [queue.Queue() for _ in range(2)]
acc_qcount = 0
acc_batch = 0

#-------------------------------------------------------------------------#
# Define Buffer to receive data from server
class DataObject:
    def __init__(self, id, ll, la, ra, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4):
        self.id = id
        self.ll = ll
        self.la = la
        self.ra = ra
        self.v1 = v1
        self.x1 = x1
        self.y1 = y1
        self.z1 = z1
        self.x2 = x2
        self.y2 = y2
        self.z2 = z2
        self.x3 = x3
        self.y3 = y3
        self.z3 = z3
        self.x4 = x4
        self.y4 = y4
        self.z4 = z4
#-------------------------------------------------------------------------#
def print_array(array, rows):
    for row in range(rows):
        print(array[row])
    
def receive_data(socket):
    # Receive binary data from the socket
    data = socket.recv(struct.calcsize("Iiiiiiiiiiiiiiiii"))
    
    # Unpack binary data into DataObject
    id, ll, la, ra, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4 = struct.unpack("Iiiiiiiiiiiiiiiii", data)
    # print('\n', id, ' ', ll, ' ', la, ' ', ra, ' ', v1, ' ', x1, ' ', y1, ' ', z1, ' ', x2, ' ', y2, ' ', z2, ' ', x3, ' ', y3, ' ', z3, ' ', x4, ' ', y4, ' ', z4,'\n')

    return DataObject(id, ll, la, ra, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4)

def read_data(data, ecg_Data, acc_Data):
    while True:
        ecg_Data.ra = data.ra
        ecg_Data.ll = data.ll
        ecg_Data.la = data.la
        ecg_Data.v1 = data.v1
        ecg_sample = [ecg_Data.ra, ecg_Data.ll, ecg_Data.la, ecg_Data.v1]
        print(ecg_sample)

        if all(x != -1 for x in [data.x1, data.y1, data.z1, data.x2, data.y2, data.z2, data.x3, data.y3, data.z3, data.x4, data.y4, data.z4]):
            acc_Data.BHI[1].x = data.x1
            acc_Data.BHI[1].y = data.y1
            acc_Data.BHI[1].z = data.z1
            acc_Data.BHI[2].x = data.x2
            acc_Data.BHI[2].y = data.y2
            acc_Data.BHI[2].z = data.z2
            acc_Data.BHI[3].x = data.x3
            acc_Data.BHI[3].y = data.y3
            acc_Data.BHI[3].z = data.z3
            acc_Data.BHI[4].x = data.x4
            acc_Data.BHI[4].y = data.y4
            acc_Data.BHI[4].z = data.z4
            acc_sample = [[acc_Data.BHI[_].x, acc_Data.BHI[_].y, acc_Data.BHI[_].z] for _ in range(1, 5)]
            print(acc_sample)

STATUS = int(input("Enter 1 for dummy data, 2 for live data: "))

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.send(struct.pack("I", STATUS))

    while True:
        data = receive_data(s)

        read_data(data, ecg_Data, acc_Data)

       
        #     ecg_Data.ra = data.data1
        #     ecg_Data.ll = data.data2
        #     ecg_Data.la = data.data3
        #     ecg_Data.v1 = data.data4
        #     ecg_sample = [ecg_Data.ra, ecg_Data.ll, ecg_Data.la, ecg_Data.v1]
        #

        #     if(ecg_index == 2999):
        #         ecg_batch = ecg_batch + 1
        #         q_ecg = ecg_qcount
        #         ecg_qcount = 1 - ecg_qcount
        #         if not ecg_queue[q_ecg].empty():
        #             ecg_Array = []
        #             Eq_len = ecg_queue[ecg_qcount].qsize()
        #             while not ecg_queue[q_ecg].empty():
        #                 ecg_Array.append(ecg_queue[q_ecg].get())
        #             print(f'ECG BATCH : {ecg_batch}')
        #             print(f'\nNumber of samples : {Eq_len}\n')
        #             print_array(ecg_Array, Eq_len)

        # elif(data.prefix == 11):
        #     acc_index = data.id % 300
        #     imu = data.data1
        #     acc_Data.BHI[imu].x = data.data2
        #     acc_Data.BHI[imu].y = data.data3
        #     acc_Data.BHI[imu].z = data.data4

        #     if(imu == 5):
        #         acc_sample = [[acc_Data.BHI[_].x, acc_Data.BHI[_].y, acc_Data.BHI[_].z] for _ in range(1, 6)]
        #         acc_queue[acc_qcount].put(acc_Data)

        #     if(acc_index == 299):
        #         acc_batch = acc_batch + 1
        #         q_acc = acc_qcount
        #         acc_qcount = 1 - acc_qcount
        #         if not acc_queue[q_acc].empty():
        #             acc_Array = []
        #             Aq_len = acc_queue[acc_qcount].qsize()
        #             while not acc_queue[q_acc].empty():
        #                 acc_Array.append(acc_queue[q_acc].get())
        #             print(f'\nECG BATCH : {acc_batch}\n')
        #             print(f'\nNumber of samples : {Aq_len}\n')
        #             print_array(acc_Array, Aq_len)

        






    # while True:
    #     l_looper = 0
        # Extracting data
        # Initialize arrays to store the data

        # prefixs = []
        # ids = []
        # data1s = []
        # data2s = []
        # data3s = []
        # data4s = []
    

        # while (l_looper<=3000):
        #     data = receive_data(s)

        #     if not data:
        #         break
            
        #     # Iterate through the dictionary and append values to arrays
        #     for key, value in data.__dict__.items():
        #         if key == 'prefix':
        #             prefixs.append(value)
        #         if key == 'id':
        #             ids.append(value)
        #         elif key == 'data1':
        #             data1s.append(value)
        #         elif key == 'data2':
        #             data2s.append(value)
        #         elif key == 'data3':
        #             data3s.append(value)
        #         elif key == 'data4':
        #             data4s.append(value)
        #     l_looper += 1       
        
    
        
