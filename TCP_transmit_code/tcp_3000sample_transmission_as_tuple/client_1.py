# print_arrays.py

import multiprocessing
import multiprocessing
import subprocess

from intermediate import ecg_Data, acc_Data

ECG_Array = []
ACC_Array = []
ecg_batch = 0
acc_batch = 0

def get_ECG_Array(queue1, queue2, switchq, filled_queue_ind):
    switchq.wait()
    ECG_Array.clear()
    with filled_queue_ind.get_locked():
        if filled_queue_ind.value == 1:
            queue = queue1
        elif filled_queue_ind.value == 2:
            queue = queue2
    ecg_batch += 1
    print(f'ECG_Batch : {ecg_batch}')
    row = 0
    while not queue.empty():
        ECG_Array.append(queue.get())
        print(ACC_Array[row])
        row += 1
    
    
def get_ACC_Array(queue1, queue2, switchq, filled_queue_ind):
    switchq.wait()
    ACC_Array.clear()
    with filled_queue_ind.get_locked():
        if filled_queue_ind.value == 1:
            queue = queue1
        elif filled_queue_ind.value == 2:
            queue = queue2
    acc_batch += 1
    print(f'ACC_Batch : {acc_batch}')
    row = 0
    while not queue.empty():
        ACC_Array.append(queue.get())
        print(ACC_Array[row])
        row += 1
    

def start_c_program():
    return subprocess.Popen(["./main"]) 

def start_intermediate():
    return subprocess.Popen(["python3", "intermediate.py"])   

if __name__ == "__main__":

    c_process = start_c_program()
    intermediate_process = start_intermediate()

    manager = multiprocessing.Manager()
    ecg_Data.queue1 = multiprocessing.Queue(maxsize=3000)
    ecg_Data.queue2 = multiprocessing.Queue(maxsize=3000)
    ecg_Data.switchq = multiprocessing.Event()
    ecg_Data.filledq = multiprocessing.Value('i', 0)

    acc_Data.queue1 = multiprocessing.Queue(maxsize=300) 
    acc_Data.queue2 = multiprocessing.Queue(maxsize=300) 
    acc_Data.switchq = multiprocessing.Event()
    acc_Data.filledq = multiprocessing.Value('i', 0)

    while True:       

        ecg_array_process = multiprocessing.Process(target=get_ECG_Array, args=(ecg_Data.queue1, ecg_Data.queue2, ecg_Data.switchq, ecg_Data.filledq,))
        # acc_array_process = multiprocessing.Process(target=get_ACC_Array, args=(acc_Data.queue1, acc_Data.queue2, acc_Data.switchq, acc_Data.filledq,))

        ecg_array_process.start()
        # acc_array_process.start()

        ecg_array_process.join()
        # acc_array_process.join()

        try:
            c_process.wait()
            intermediate_process.wait()
        except KeyboardInterrupt:
            c_process.terminate()
            intermediate_process.terminate()
            ecg_array_process.terminate()
            # acc_array_process.terminate()
        

        
