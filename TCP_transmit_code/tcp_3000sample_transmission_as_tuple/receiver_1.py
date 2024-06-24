
import socket
import struct
import subprocess
import multiprocessing
import pandas as pd

HOST = '127.0.0.1'
PORT = 6789

subprocess.Popen(["./main"])

#------------------------------------------------------------------------------------#
# 6s ECG buffer 
class ECG:

    def __init__(self, qcount=0, batch=0):
        self.ADS = self.sensor(0, 0, 0, 0)
        self.ecg_sample = self.ADS.sample
        self.qcount = qcount
        self.batch = batch

    class sensor:
        def __init__(self, ra=0, ll=0, la=0, v1=0):
            self.ra = ra
            self.ll = ll
            self.la = la
            self.v1 = v1
            self.sample = [self.ra, self.ll, self.la, self.v1]

    def store_data(self, queue1, queue2, switchq, filled_queue_ind):
        current_queue = queue1
        if current_queue.full():
            with filled_queue_ind.get_lock():
                filled_queue_ind.value = 1 if current_queue == queue1 else 2
            switchq.set()
            current_queue = queue2 if current_queue == queue1 else queue2
            self.batch += 1
            switchq.clear()
        current_queue.put(self.ecg_sample)

ecg_Data = ECG()

#----------------------------------------------------------------------------------------------#
# 6s ACC buffer
class ACC:
    def __init__(self, qcount=0, batch=0):
        self.BHI = {i: self.sensor(i, 0, 0, 0) for i in range(1, 5)}  
        self.acc_sample = [self.BHI[_].sample for _ in range(1, 5)]
        self.qcount = qcount
        self.batch = batch

    class sensor:
        def __init__(self, imu_num=0, x=0, y=0, z=0):
            self.imu_num = imu_num
            self.x = x
            self.y = y
            self.z = z
            self.sample = [self.x, self.y, self.z]

    def store_data(self, queue1, queue2, switchq, filled_queue_ind):
        current_queue = queue1
        if current_queue.full():
            with filled_queue_ind.get_lock():
                filled_queue_ind.value = 1 if current_queue == queue1 else 2
            switchq.set()
            current_queue = queue2 if current_queue == queue1 else queue2
            self.batch += 1
            switchq.clear()
        current_queue.put(self.acc_sample)

acc_Data = ACC() 
#-----------------------------------------------------------------------------------------------#
# Define Buffer to receive data from server
class DataObject:
    def __init__(self, id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4):
        self.id = id
        self.ra = ra
        self.ll = ll
        self.la = la
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


def receive_data(socket):
    data = socket.recv(struct.calcsize("Iiiiiiiiiiiiiiiii"))
    id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4 = struct.unpack("Iiiiiiiiiiiiiiiii", data)
    #print('\n', id, ' ', ra, ' ', ll, ' ', la, ' ', v1, ' ', x1, ' ', y1, ' ', z1, ' ', x2, ' ', y2, ' ', z2, ' ', x3, ' ', y3, ' ', z3, ' ', x4, ' ', y4, ' ', z4, '\n')
    return DataObject(id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4)

        
def read_data(data, ecg_Data, acc_Data):
    # ecg_Data.ADS.ra = data.ra
    # ecg_Data.ADS.ll = data.ll
    # ecg_Data.ADS.la = data.la
    # ecg_Data.ADS.v1 = data.v1
    ecg_Data.ecg_sample = [data.ra, data.ll, data.la, data.v1]
    print(ecg_Data.ecg_sample)

    if all(x != -1 for x in [data.x1, data.y1, data.z1, data.x2, data.y2, data.z2, data.x3, data.y3, data.z3, data.x4, data.y4, data.z4]):
        # acc_Data.BHI[1].x = data.x1
        # acc_Data.BHI[1].y = data.y1
        # acc_Data.BHI[1].z = data.z1
        # acc_Data.BHI[2].x = data.x2
        # acc_Data.BHI[2].y = data.y2
        # acc_Data.BHI[2].z = data.z2
        # acc_Data.BHI[3].x = data.x3
        # acc_Data.BHI[3].y = data.y3
        # acc_Data.BHI[3].z = data.z3
        # acc_Data.BHI[4].x = data.x4
        # acc_Data.BHI[4].y = data.y4
        # acc_Data.BHI[4].z = data.z4
        acc_Data.acc_sample = [[data.x1, data.y1, data.z1], [data.x2, data.y2, data.z2], [data.x3, data.y3, data.z3], [data.x4, data.y4, data.z4]] 
        print(acc_Data.acc_sample)
        

#-----------------------------------------------------------------------------------------------#

if __name__ == "__main__":

    STATUS = int(input("Enter 1 for dummy data, 2 for live data: "))


    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.send(struct.pack("I", STATUS))

        while True:
            data = receive_data(s)
            read_data(data, ecg_Data, acc_Data)


