
import socket
import struct
import subprocess
import queue

HOST = '127.0.0.1'
PORT = 6789

subprocess.Popen(["./main"])

#------------------------------------------------------------------------------------#
class Sensor_Data:

    def __init__(self, E_queue1, A_queue1):
        self.ecg_Data = self.ECG()
        self.acc_Data = self.ACC()
        self.as_Data = self.AS()
        self.E_batch = 0
        self.A_batch = 0
        self.E_queue1 = E_queue1
        self.A_queue1 = A_queue1 

    class ECG:

        def __init__(self):
            self.ADS = self.sensor(0, 0, 0, 0)
            self.ecg_sample = self.ADS.sample

        class sensor:
            def __init__(self, ra=0, ll=0, la=0, v1=0):
                self.ra = ra
                self.ll = ll
                self.la = la
                self.v1 = v1
                self.sample = [self.ra, self.ll, self.la, self.v1]

    class ACC:

        def __init__(self, qcount=0, batch=0):
            self.BHI = {i: self.sensor(i, 0, 0, 0) for i in range(1, 5)}  
            self.acc_sample = [self.BHI[_].sample for _ in range(1, 5)]  

        class sensor:
            def __init__(self, imu_num=0, x=0, y=0, z=0):
                self.imu_num = imu_num
                self.x = x
                self.y = y
                self.z = z
                self.sample = [self.x, self.y, self.z]

    class AS:

        def __init__(self):
            self.ADS_as = self.sensor(0, 0, 0, 0)
            self.as_sample = self.ADS_as.sample

        class sensor:
            def __init__(self, as1=0, as2=0, as3=0, as4=0):
                self.as1 = as1
                self.as2 = as2
                self.as3 = as3
                self.as4 = as4
                self.sample = [self.as1, self.as2, self.as3, self.as4]

    class DataObject:

        def __init__(self, id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, as1, as2, as3, as4):
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
            self.as1 = as1
            self.as2 = as2
            self.as3 = as3
            self.as4 = as4

    def receive_data(self, socket):

        data = socket.recv(struct.calcsize("Iiiiiiiiiiiiiiiiiiiii"))
        data_tuple=struct.unpack("Iiiiiiiiiiiiiiiiiiiii", data)
        print(data_tuple)
        id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, as1, as2, as3, as4 = struct.unpack("Iiiiiiiiiiiiiiiiiiiii", data)
        # print("Data received successfully!")
        # print('\n', id, ' ', ra, ' ', ll, ' ', la, ' ', v1, ' ', x1, ' ', y1, ' ', z1, ' ', x2, ' ', y2, ' ', z2, ' ', x3, ' ', y3, ' ', z3, ' ', x4, ' ', y4, ' ', z4, ' ', as1, ' ', as2, ' ', as3, ' ', as4 ,'\n')
        return self.DataObject(id, ra, ll, la, v1, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, as1, as2, as3, as4)
    

    def Process_Data(self):

        # STATUS = int(input("Enter 1 for dummy data, 2 for live data: "))
        
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            # s.send(struct.pack("I", STATUS))

            while True:

                data = self.receive_data(s)

                self.ecg_Data.ADS.ra = data.ra
                self.ecg_Data.ADS.ll = data.ll
                self.ecg_Data.ADS.la = data.la
                self.ecg_Data.ADS.v1 = data.v1
                self.ecg_Data.ecg_sample = [data.ra, data.ll, data.la, data.v1]
                # print(self.ecg_Data.ecg_sample)

                # if not self.E_queue1.full():
                #     self.E_queue1.put(self.ecg_Data.ecg_sample)
                

                self.as_Data.ADS_as.as1 = data.as1
                self.as_Data.ADS_as.as2 = data.as2
                self.as_Data.ADS_as.as3 = data.as3
                self.as_Data.ADS_as.as4 = data.as4
                

                if all(x != -1 for x in [data.x1, data.y1, data.z1, data.x2, data.y2, data.z2, data.x3, data.y3, data.z3, data.x4, data.y4, data.z4]):
                    self.acc_Data.BHI[1].x = data.x1
                    self.acc_Data.BHI[1].y = data.y1
                    self.acc_Data.BHI[1].z = data.z1
                    self.acc_Data.BHI[2].x = data.x2
                    self.acc_Data.BHI[2].y = data.y2
                    self.acc_Data.BHI[2].z = data.z2
                    self.acc_Data.BHI[3].x = data.x3
                    self.acc_Data.BHI[3].y = data.y3
                    self.acc_Data.BHI[3].z = data.z3
                    self.acc_Data.BHI[4].x = data.x4
                    self.acc_Data.BHI[4].y = data.y4
                    self.acc_Data.BHI[4].z = data.z4
                    self.acc_Data.acc_sample = [[data.x1, data.y1, data.z1], [data.x2, data.y2, data.z2], [data.x3, data.y3, data.z3], [data.x4, data.y4, data.z4]] 
                    # print(self.acc_Data.acc_sample)

                    # if not self.A_queue1.full():
                    #     self.A_queue1.put(self.acc_Data.acc_sample)
                        
    def get_ecg(self):
        if self.E_queue1.full():
            j = 0
            while not self.E_queue1.empty():
                j+=1
                print(f"{j} : ", self.E_queue1.get())
                self.E_batch += 1  
            print(f'ECG batch : {self.E_batch}')

    def get_acc(self):
        if self.A_queue1.full():
            j = 0
            while not self.A_queue1.empty():
                j+=1
                print(f"{j} : ", self.A_queue1.get())
                self.A_batch += 1  
            print(f'ACC batch : {self.A_batch}')
            
                        
                   
if __name__ == "__main__":

    E_queue1 = queue.Queue(maxsize=3000)
    A_queue1 = queue.Queue(maxsize=300)
    sensor = Sensor_Data(E_queue1, A_queue1)
    sensor.Process_Data()
    # sensor.get_acc
    # sensor.get_ecg
