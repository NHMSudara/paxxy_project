import socket
import struct
import subprocess
import queue

HOST = '127.0.0.1'
PORT = 6789

subprocess.Popen(["./main"])

class Sensor_Data:
    def __init__(self, E_queue1):
        self.ecg_Data = self.ECG()
        self.acc_Data = self.ACC()
        self.as_Data = self.AS()
        self.E_batch = 0
        self.A_batch = 0
        self.E_queue1 = E_queue1

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

    def receive_data(self, sock):
        format_str = "20s"
        expected_length = struct.calcsize(format_str)

        def recv_all(sock, length):
            data = b''
            while len(data) < length:
                more = sock.recv(length - len(data))
                if not more:
                    raise EOFError('Socket closed before receiving all data')
                data += more
            return data

        data = recv_all(sock, expected_length)
        data_tuple = struct.unpack(format_str, data)
        received_string = data_tuple[0].decode('utf-8').rstrip('\x00')
        return received_string

    def Process_Data(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))

            while True:
                data = self.receive_data(s)
                self.E_queue1.put(data)
            return 0

if __name__ == "__main__":
    E_queue1 = queue.Queue(maxsize=3000)
    sensor = Sensor_Data(E_queue1)
    sensor.Process_Data()