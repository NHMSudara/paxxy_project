# print_arrays.py
import multiprocessing
import subprocess

from intermediate import Sensor_Data

ECG_Array = []
ACC_Array = []



def main():

    # subprocess.Popen(["python3", "intermediate.py"])  
 
    E_queue1 = multiprocessing.Queue(maxsize=3000)


    A_queue1 = multiprocessing.Queue(maxsize=300)

    ecg_batch = 0
    acc_batch = 0 

    Data_Process = Sensor_Data(E_queue1, A_queue1)
    process = multiprocessing.Process(target=Data_Process.Process_Data)

    process.start()

    try:
        while True:
            ECG_Array.append(E_queue1.get())
            if len(ECG_Array) >= 3000:
                ecg_batch += 1
                print(f'ECG batch : {ecg_batch}')
                # for i, E_row in enumerate(ECG_Array):
                #     print(f'{i} :   {E_row}')
                E_queue1.empty()
                ECG_Array.clear()
                
            ACC_Array.append(A_queue1.get())
            if len(ACC_Array) >= 300:
                acc_batch += 1
                print(f'ACC batch : {acc_batch}')
                # for j, A_row in enumerate(ACC_Array):
                #     print(f'{j} :   {A_row}')
                A_queue1.empty()
                ACC_Array.clear()
                
    except KeyboardInterrupt:
        pass
    
    process.join()
    print(f"Number of ECG batches printed : {ecg_batch}")
    print(f"Number of ACC batches printed : {acc_batch}")
    print("Goodbye!")

    
if __name__ == "__main__":

    main()








        

        
