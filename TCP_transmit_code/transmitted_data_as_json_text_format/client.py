# print_arrays.py
import multiprocessing
import subprocess

from intermediate import Sensor_Data

def get_sensor_data(queue):
    id = []
    ecg_1 = []
    ecg_2 = []
    ecg_3 = []
    ecg_4 = []
    imu_en = 0
    acc_x1 = []
    acc_y1 = []
    acc_z1 = []
    acc_x2 = []
    acc_y2 = []
    acc_z2 = [] 
    acc_x3 = []
    acc_y3 = []
    acc_z3 = []
    acc_x4 = []
    acc_y4 = []
    acc_z4 = [] 
    as_1 = []
    as_2 = []
    as_3 = []
    as_4 = [] 
    while(1):
        if not queue.empty():    
            data = queue.get()
            id.append(data["id"])
            ecg_1.append(int(data["ecg_1"]))
            ecg_2.append(data["ecg_2"])
            ecg_3.append(data["ecg_3"])
            ecg_4.append(data["ecg_4"])
            imu_en = int(data["imu_en"])
            if imu_en == 1:
                acc_x1.append(data["acc_x1"])
                acc_y1.append(data["acc_y1"])
                acc_z1.append(data["acc_z1"])
                acc_x2.append(data["acc_x2"])
                acc_y2.append(data["acc_y2"])
                acc_z2.append(data["acc_z2"])
                acc_x3.append(data["acc_x3"])
                acc_y3.append(data["acc_y3"])
                acc_z3.append(data["acc_z3"])
                acc_x4.append(data["acc_x4"])
                acc_y4.append(data["acc_y4"])
                acc_z4.append(data["acc_z4"])
                as_1.append(data["as_1"])
                as_2.append(data["as_2"])
                as_3.append(data["as_3"])
                as_4.append(data["as_4"])
        if len(ecg_1) == 3000:
            return   id,ecg_1,ecg_2,ecg_3,ecg_4,acc_x1,acc_y1,acc_z1,acc_x2,acc_y2,acc_z2,acc_x3,acc_y3,acc_z3,acc_x4,acc_y4,acc_z4,as_1,as_2,as_3,as_4
    
def main():
    queue = multiprocessing.Queue()
    Data_Process = Sensor_Data(queue)
    process = multiprocessing.Process(target=Data_Process.Process_Data)
    process.start()
    batch_num=1

    try:
        while True:
            
            id,ecg_1,ecg_2,ecg_3,ecg_4,acc_x1,acc_y1,acc_z1,acc_x2,acc_y2,acc_z2,acc_x3,acc_y3,acc_z3,acc_x4,acc_y4,acc_z4,as_1,as_2,as_3,as_4 = get_sensor_data(queue)
            print(f"----batch number: {batch_num}-----")
            print("----ecg_1 data-----")
            print(ecg_1)
            print("----acc_z1 data------")
            print(as_4)
            print(len(as_4))
            batch_num+=1
          
    except KeyboardInterrupt:
        pass
    

    
if __name__ == "__main__":

    main()


        

        
