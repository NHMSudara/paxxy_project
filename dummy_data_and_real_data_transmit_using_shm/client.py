#python 
# Install sysv_ipc module firstly if you don't have this
import time
import sysv_ipc as ipc
import struct


class ECGData:
    def __init__(ads1298, ra=0, ll=0, la=0, v1=0):
        ads1298.ra = ra #E1
        ads1298.v1 = v1 #E2
        ads1298.la = la #E3
        ads1298.ll = ll #E4
    # def __repr__(ads1298):
    #     return f"ECGData(ra={ads1298.ra}, ll={ads1298.ll}, la={ads1298.la}, v1={ads1298.v1})"

class AccSensorData: 
    def __init__(bhi260ap, x=0, y=0, z=0):
        bhi260ap.x = x 
        bhi260ap.y = y
        bhi260ap.z = z
    # def __repr__(bhi260ap):
    #     return f"AccSensorData(x={bhi260ap.x}, y={bhi260ap.y}, z={bhi260ap.z})"

class AccData:
    def __init__(acc):
        acc.sensor = [AccSensorData() for _ in range(5)]  # 4 imu sensors 
    # def __repr__(acc):
    #     return f"AccData(sensor={acc.sensor})"


ecg_data_array = [ECGData() for _ in range(3000)]
acc_data_array = [AccData() for _ in range(300)]



# Create an array of 3000 rows of ['E1', 'E2', 'E3', 'E4']
#ecg_array = [[None for _ in range(4)] for _ in range(3000 -1)]

# Create an array of 300 rows of ['TOP', 'MID', 'RIGHT', 'LEFT']
#acc_array = [[None for _ in range(12)] for _ in range(300 -1)]



def main():
    path = "/tmp"
    key = ipc.ftok(path, 2345, silence_warning =True)
    shm = ipc.SharedMemory(key, 0, 0)
    #I found if we do not attach ourselves
    #it will attach as ReadOnly.
    shm.attach(0,0) 
    
    try:

        buffer_0_incremented_printed = False
        buffer_1_incremented_printed = False

        while True:
            #reading buffer 0 batch numbers	
            #buf = shm.read(8)
            #ecg, acc = struct.unpack("ii",buf)

            buffer_0_batch_number = int.from_bytes(shm.read(4), byteorder='little', signed=False)
            buffer_1_batch_number = int.from_bytes(shm.read(4, 62408), byteorder='little', signed=False)

            time.sleep(0.1)

            new_buffer_0_batch_number = int.from_bytes(shm.read(4, 62408), byteorder='little', signed=False)
            new_buffer_1_batch_number = int.from_bytes(shm.read(4), byteorder='little', signed=False)



            if new_buffer_0_batch_number == buffer_0_batch_number + 1 and not buffer_0_incremented_printed:
                #print("Buffer 0 incremented")
                #buffer_0_incremented_printed = True
                offset = 8
                for i in range(0, 2999):
                    buf = shm.read(16,offset+(16*i))
                    ecg_data_array[i].ra, ecg_data_array[i].v1, ecg_data_array[i].la,  ecg_data_array[i].ll = struct.unpack("iiii",buf)

                offset = 8 + 16*3000
                for i in range(0, 299):
                    buf = shm.read(48,offset+(48*i))
                    acc_data_array[i].sensor[0].x, acc_data_array[i].sensor[0].y, acc_data_array[i].sensor[0].z, acc_data_array[i].sensor[1].x, acc_data_array[i].sensor[1].y, acc_data_array[i].sensor[1].z, acc_data_array[i].sensor[2].x, acc_data_array[i].sensor[2].y, acc_data_array[i].sensor[2].z, acc_data_array[i].sensor[3].x, acc_data_array[i].sensor[3].y, acc_data_array[i].sensor[3].z = struct.unpack("iiiiiiiiiiii",buf)

            if new_buffer_1_batch_number == buffer_1_batch_number + 1 and not buffer_1_incremented_printed:
                #print("Buffer 1 incremented")
                #buffer_1_incremented_printed = True
                offset = 62408+8
                for i in range(0, 2999):
                    buf = shm.read(16,offset+(16*i))
                    ecg_data_array[i].ra, ecg_data_array[i].v1, ecg_data_array[i].la, ecg_data_array[i].ll = struct.unpack("iiii",buf)
                offset = 62408+8 + 16*3000
                for i in range(0, 299):
                    buf = shm.read(48,offset+(48*i))
                    acc_data_array[i].sensor[0].x, acc_data_array[i].sensor[0].y, acc_data_array[i].sensor[0].z, acc_data_array[i].sensor[1].x, acc_data_array[i].sensor[1].y, acc_data_array[i].sensor[1].z, acc_data_array[i].sensor[2].x, acc_data_array[i].sensor[2].y, acc_data_array[i].sensor[2].z, acc_data_array[i].sensor[3].x, acc_data_array[i].sensor[3].y, acc_data_array[i].sensor[3].z= struct.unpack("iiiiiiiiiiii",buf)
                

            #print the array once if batch nmber incremented
            if new_buffer_0_batch_number == buffer_0_batch_number + 1 and not buffer_0_incremented_printed or new_buffer_1_batch_number == buffer_1_batch_number + 1 and not buffer_1_incremented_printed:
                # Print ECG data
                print("ECG Data:")
                for i in range(0, 2999):
                    print(f"RA: {ecg_data_array[i].ra}, V1: {ecg_data_array[i].v1}, LA: {ecg_data_array[i].la}, LL: {ecg_data_array[i].ll}")

                # Print Acc data
                print("\nAccelerometer Data:")
                for i in range(0, 299):
                    print(f"S1={acc_data_array[i].sensor[0].x, acc_data_array[i].sensor[0].y, acc_data_array[i].sensor[0].z}, S2={acc_data_array[i].sensor[1].x, acc_data_array[i].sensor[1].y, acc_data_array[i].sensor[1].z}, S3={acc_data_array[i].sensor[2].x, acc_data_array[i].sensor[2].y, acc_data_array[i].sensor[2].z}, S4={acc_data_array[i].sensor[3].x, acc_data_array[i].sensor[3].y, acc_data_array[i].sensor[3].z}")

                buffer_0_incremented_printed = True
                buffer_1_incremented_printed = True

            # Reset the flags if no increment is detected to allow printing again
            if new_buffer_0_batch_number != buffer_0_batch_number + 1:
                buffer_0_incremented_printed = False

            if new_buffer_1_batch_number != buffer_1_batch_number + 1:
                buffer_1_incremented_printed = False


    finally:        
        shm.detach()    
    #   pass


if __name__ == '__main__':
    main()
 
