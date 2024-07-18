import socket
import struct
import queue
import json
import subprocess
import multiprocessing
import time

HOST = '127.0.0.1'
PORT = 8080

class Sensor_Data:
    def __init__(self, queue):
        self.queue = queue

    def Process_Data(self):
        BUFFER_SIZE = 1024
        DELIMITER = '\n'
        count = 0
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((HOST, PORT))
            s.listen()
            print(f"Server listening on {HOST}:{PORT}")
            subprocess.Popen(["./main_1"])
            time.sleep(5)
            data_batch = []
            while True:
                conn, addr = s.accept()
                with conn:
                    print(f"Connected by {addr}")
                    buffer = ''
                    while True:
                        data = conn.recv(BUFFER_SIZE)
                        if not data:
                            print("No Data received!")
                            break
                        buffer += data.decode('utf-8')
                        while DELIMITER in buffer:
                            json_str, buffer = buffer.split(DELIMITER, 1)
                            try:
                                data_json = json.loads(json_str)
                                #data_batch.append(data_json)  # Store parsed JSON
                                #if len(data_batch) >= 3000:
                                    # Example of accessing data
                                self.queue.put(data_json)  
                                # print(f'ECG batch: {data_batch}')
                                #data_batch.clear()
                            except json.JSONDecodeError:
                                pass
                                # print("Received data is not valid JSON")

# if __name__ == "__main__":
#     queue = queue.Queue()
#     sensor = Sensor_Data(queue)
#     sensor.Process_Data()
