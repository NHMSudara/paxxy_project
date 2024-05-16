#python 

# Install sysv_ipc module firstly if you don't have this
import sysv_ipc as ipc
import struct

def main():
    path = "/tmp"
    key = ipc.ftok(path, 2345, silence_warning =True)
    shm = ipc.SharedMemory(key, 0, 0)

    #I found if we do not attach ourselves
    #it will attach as ReadOnly.
    shm.attach(0,0)  

	#reading buffer 0 batch numbers	
    buf = shm.read(8)
    ecg, acc = struct.unpack("ii",buf)
    print(ecg)
    print(acc)
    
    	#reading buffer 0 ecg data
    offset = 8
    for i in range(0, 2999):
    	buf = shm.read(16,offset+(16*i))
    	ra,ll,la,v1 = struct.unpack("iiii",buf)
    	
    	#reading buffer 0 acc data
    offset = 8 + 16*3000
    for i in range(0, 299):
    	buf = shm.read(48,offset+(48*i))
    	s1x, s1y, s1z, s2x, s2y, s2z, s3x, s3y, s3z, s4x, s4y, s4z  = struct.unpack("iiiiiiiiiiii",buf)
    				    	
    	#reading buffer 1 batch number
    buf = shm.read(8, 62408)
    ecg, acc = struct.unpack("ii",buf)
    print(ecg)
    print(acc)

	#reading buffer 1 ecg data
    offset = 62408+8
    for i in range(0, 2999):
    	buf = shm.read(16,offset+(16*i))
    	ra,ll,la,v1 = struct.unpack("iiii",buf)
    
    	#reading buffer 1 acc data
    offset = 62408+8 + 16*3000
    for i in range(0, 299):
    	buf = shm.read(48,offset+(48*i))
    	s1x, s1y, s1z, s2x, s2y, s2z, s3x, s3y, s3z, s4x, s4y, s4z  = struct.unpack("iiiiiiiiiiii",buf)
    
    shm.detach()
    pass



if __name__ == '__main__':
    main()
 
