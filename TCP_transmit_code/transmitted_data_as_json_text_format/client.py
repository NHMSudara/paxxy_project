# print_arrays.py
import multiprocessing
import subprocess

from intermediate import Sensor_Data

def get_sensor_data(queue):
    array = []
    while(1)
        if not queue.empty():
            array.append(queue.get())
        if len(array) == 3000:
            return array   


def main():
    queue = multiprocessing.Queue()

    Data_Process = Sensor_Data(queue)

    process = multiprocessing.Process(target=Data_Process.Process_Data)

    process.start()

    try:
        while True:
            Array=get_sensor_data(queue)
            print(Array)
          
    except KeyboardInterrupt:
        pass
    

    
if __name__ == "__main__":

    main()


        

        
