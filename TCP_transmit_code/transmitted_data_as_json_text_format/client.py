# print_arrays.py
import multiprocessing
import subprocess

from intermediate import Sensor_Data

ECG_Array = []



def main():

    subprocess.Popen(["python3", "intermediate.py"])  
 
    E_queue1 = multiprocessing.Queue(maxsize=3500)




    ecg_batch = 0
    

    Data_Process = Sensor_Data(E_queue1)
    process = multiprocessing.Process(target=Data_Process.Process_Data)

    process.start()

    try:
        while True:
            ECG_Array.append(E_queue1.get())
            if len(ECG_Array) >= 3000:
                
                print(f'ECG batch : {ECG_Array[0]["ecg"]}')
                # for i, E_row in enumerate(ECG_Array):
                #     print(f'{i} :   {E_row}')
                E_queue1.empty()
                ECG_Array.clear()
                
                
    except KeyboardInterrupt:
        pass
    

 

    
if __name__ == "__main__":

    main()








        

        
