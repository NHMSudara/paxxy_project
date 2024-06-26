# print_arrays.py
import multiprocessing
import subprocess

from intermediate import Sensor_Data

def main():


 
    queue = multiprocessing.Queue()

    Data_Process = Sensor_Data(queue)

    process = multiprocessing.Process(target=Data_Process.Process_Data)

    process.start()

    try:
        while True:
            if not queue.empty():
                Array = queue.get(0:3)
                print(Array)
                print(type(Array))
                print(f'successfully Printed !!!!!!!!!!!')


            
                
    except KeyboardInterrupt:
        pass
    

    
if __name__ == "__main__":

    main()








        

        
