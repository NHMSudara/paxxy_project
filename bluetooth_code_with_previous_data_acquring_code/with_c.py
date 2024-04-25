###################################################################
# Imports for signal processing
import numpy as np
#from scipy import signal
import pandas as pd
from sklearn.decomposition import FastICA
from scipy.signal import lfilter
from pyentrp import entropy as ent
import csv
import time
#import cProfile, pstats, io
#from pstats import SortKey

###################################################################
# Data receiving from c file
import socket
import struct
HOST = '127.0.0.1'
PORT = 12345

# Define the DataObject structure in Python
class DataObject:
    def __init__(self, id, ra, ll,la,v1,as1,as2,as3,as4):
        self.id = id
        self.ra = ra
        self.ll= ll
        self.la = la
        self.v1 =v1
        self.as1 = as1
        self.as2 = as2
        self.as3 = as3
        self.as4 = as4

def receive_data(socket):
    # Receive binary data from the socket
    data = socket.recv(struct.calcsize("Iffffiiii"))
    # print("Received data length:", len(data))  
    # Unpack binary data into DataObject
    id, ra, ll, la,v1, as1, as2, as3, as4 = struct.unpack("Iffffiiii", data)

    return DataObject(id, ra, ll, la, v1,as1, as2, as3, as4)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    while True:
        l_looper = 0

        ids = []
        ras = []
        lls = []
        las = []
        v1s = []
        as1s = []
        as2s = []
        as3s = []
        as4s = []


        while (l_looper<=3000):
            data = receive_data(s)
            if not data:
                break
            # Process the received data in real-time
            start_time = time.time()
            #print(data.__dict__)
            # Iterate through the dictionary and append values to arrays
            for key, value in data.__dict__.items():
                if key == 'id':
                    ids.append(value)
                elif key == 'ra':
                    ras.append(value)
                elif key == 'll':
                    lls.append(value)
                elif key == 'la':
                    las.append(value)
                elif key == 'v1':
                    v1s.append(value)    
                elif key == 'as1':
                    as1s.append(value)
                elif key == 'as2':
                    as2s.append(value)
                elif key == 'as3':
                    as3s.append(value)
                elif key == 'as4':
                    as4s.append(value)
            l_looper += 1       

        

        # Extract the ECG signal columns
        ecg_signal_noisy = v1s  
        ecg_signal_ch2_noisy = ras
        ecg_signal_ch3_noisy = las

        #####################################################################
        # Code for signal processing

        #np.float = float

        # Extracting data
        # Initialize arrays to store the data
        
        # Extract the Acoustic signal columns
        top_left = as1s
        top_right = as2s
        bottom_left = as3s
        bottom_right = as4s

        # Filters
        b = pd.read_csv('data/firhigh.csv').to_numpy().flatten()
        b2 = pd.read_csv('data/firfillow.csv').to_numpy().flatten()

        fil = pd.read_csv('data/iirnotch.csv', header=None)
        num = fil.iloc[:, 0].to_numpy().flatten()
        den = fil.iloc[:, 1].to_numpy().flatten()

        #parameters
        fs = 500 #Hz
        dt = 1/fs #s

        # Specify the CSV file path
        csv_file_path = 'output_fhr_100-170.csv'
        csv_file_path_maternal = 'output_mhr.csv'

        with open(csv_file_path, 'w', newline='') as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(['Mean Original', 'STD Original', 'Median Original', 'Mean Outlier removed', 'STD Outlier removed', 'Median Outlier removed'])

        with open(csv_file_path_maternal, 'w', newline='') as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(['Mean', 'STD', 'Median'])

        def adt_findrpeaks(ecg_signal, threshold_ratio=0.4, refractory_period =150, integration_window =35):
            # Differentiation
            differentiated_signal = np.diff(ecg_signal)

            # Squaring
            squared_signal = differentiated_signal ** 2

            # Integration
            integrated_signal = np.convolve(squared_signal, np.ones(integration_window)/integration_window, mode='same')

            # Calculate adaptive thresholds
            high_threshold = threshold_ratio * np.max(integrated_signal)

            # QRS Detection
            r_indices = []
            in_refractory_period = False

            for i, value in enumerate(integrated_signal):
                if value > high_threshold and not in_refractory_period:
                    r_indices.append(i)
                    in_refractory_period = True

                if in_refractory_period and i - r_indices[-1] >= refractory_period:
                    in_refractory_period = False
                    
            return r_indices

        
        def easy_findrpeaks(ecg_signal, threshold_ratio=0.6, refractory_period =70, integration_window =30):
            # Calculate adaptive thresholds
            high_threshold = threshold_ratio * np.max(ecg_signal)

            # QRS Detection
            r_indices = []
            in_refractory_period = False

            for i, value in enumerate(ecg_signal):
                if value > high_threshold and not in_refractory_period:
                    r_indices.append(i)
                    in_refractory_period = True

                if in_refractory_period and i - r_indices[-1] >= refractory_period:
                    in_refractory_period = False
                    
            return r_indices, high_threshold
        
        def correct_sign(signal):
            #signal should be a numpy array
            peak = np.max(signal)
            trough = np.min(signal)
            if np.abs(peak)< np.abs(trough):
                return -1*signal
            else:
                return signal

        hrmedian =[]
        
        def get_hrlis(signal, threshold_ratio, refractory_period):
            # r_indices, integrated_signal, ht = adt_findrpeaks(signal)
            r_indices= adt_findrpeaks(signal, threshold_ratio = threshold_ratio, refractory_period=refractory_period)
            delta_lis = []

            for i in range(1,(len(r_indices))):
                delta_lis.append(r_indices[i] - r_indices[i-1])

            delta_t = [x*dt for x in delta_lis]
            hr_bpm = [60/x for x in delta_t]
            hr_vals = hr_bpm.copy()
            
            #My stuff starts here
            #for val in hr_vals:
            #    if val>170 or val <110:
            #        hr_vals.remove(val)

            #rang = ran.split('-')
            #start = int(rang[0].strip())
            #rem =[]

            #if start !=0:
            #for i in range(1,len(hr_vals)):
            #    if abs(hr_vals[i] - hr_vals[i-1])>=20:
            #        rem.append(i)
            
            #rem.reverse()
            
            #for pos in rem:
            #    hr_vals.pop(pos)
            
            with open(csv_file_path, 'a', newline='') as csv_file:
                writer = csv.writer(csv_file)
                mean_og = int(np.mean(np.array(hr_bpm)))
                median_og = int(np.median(np.array(hr_bpm)))
                std_og = int(np.std(np.array(hr_bpm)))

                mean_out = int(np.mean(np.array(hr_vals)))
                median_out = int(np.median(np.array(hr_vals)))
                std_out = int(np.std(np.array(hr_vals)))

                writer.writerow([mean_og, std_og, median_og, mean_out, std_out, median_out])   
            

            print('fHR = ', median_out)

        def get_hrlis_mat(signal, threshold_ratio, refractory_period):
            # r_indices, integrated_signal, ht = adt_findrpeaks(signal)
            r_indices= adt_findrpeaks(signal, threshold_ratio = threshold_ratio, refractory_period=refractory_period)
            delta_lis = []

            for i in range(1,(len(r_indices))):
                delta_lis.append(r_indices[i] - r_indices[i-1])

            delta_t = [x*dt for x in delta_lis]
            hr_bpm = [60/x for x in delta_t]

            with open(csv_file_path_maternal, 'a', newline='') as csv_file:
                writer = csv.writer(csv_file)
                mean_og = int(np.mean(np.array(hr_bpm)))
                median_og = int(np.median(np.array(hr_bpm)))
                std_og = int(np.std(np.array(hr_bpm)))
                writer.writerow([mean_og, std_og, median_og])

            print('mHR = ', median_og)

        #select a portion of the stable part of the ecg
        ecg_signal = lfilter(b2,[1], lfilter(b,[1],ecg_signal_noisy))[1500:]
        ecg_signal_ch2 = -1*lfilter(b2,[1], lfilter(b,[1],ecg_signal_ch2_noisy))[1500:]
        ecg_signal_ch3 = -1*lfilter(b2,[1], lfilter(b,[1],ecg_signal_ch3_noisy))[1500:]

        ecg_signal = lfilter(num, den, ecg_signal)
        ecg_signal_ch2 = lfilter(num, den, ecg_signal_ch2)
        ecg_signal_ch3 = lfilter(num, den, ecg_signal_ch3)
        r_indices_ori = adt_findrpeaks(ecg_signal)

        #get the maternal peaks and convert to bpm
        delta_lis = []
        dt = 1/fs
        r_len = len(r_indices_ori)
        for i in range(1,r_len):
            delta_lis.append(r_indices_ori[i] - r_indices_ori[i-1])

        delta_t = [x*dt for x in delta_lis]
        hr_bpm_maternal = [60/x for x in delta_t]
        maternal_mean = np.mean(np.array(hr_bpm_maternal))
        maternal_std = np.std(np.array(hr_bpm_maternal))
        
        
        # Extract the main ECG signal using QRS complex locations
        main_signal = np.zeros(len(ecg_signal))
        qrs_width = 24  # Adjust this value based on the width of the QRS complex
        alpha,beta = 0.65, 1.5

        for idx in r_indices_ori:
            main_signal[idx - int(alpha*qrs_width): idx + int(beta*qrs_width)] = ecg_signal[idx - int(alpha*qrs_width): idx + int(beta*qrs_width)]

        t_wave = np.zeros(len(ecg_signal))
        t_width = 35  # Adjust this value based on the width of the T wave
        gamma,delta = 0.5, 1.8
        for idx in r_indices_ori:
            for i in range(len(r_indices_ori)-1):
                idx = round(r_indices_ori[i] +(1/3)*(r_indices_ori[i+1]-r_indices_ori[i]))
                t_wave[idx - int(gamma*t_width): idx + int(delta*t_width)] = ecg_signal[idx - int(gamma*t_width): idx + int(delta*t_width)]

        # Combine the main signal with the residual signal
        residual_signal_1 = ecg_signal - main_signal
        residual_signal = residual_signal_1 - t_wave

        # Interpolate zero values in residual_signal_1 using adjacent values from t_wave
        zero_indices = np.where(residual_signal == 0)[0]  # Find indices where residual_signal_1 is zero

        for idx in zero_indices:
            left_idx = idx - 1
            right_idx = idx + 1

            # Find nearest non-zero values in t_wave for interpolation
            while left_idx >= 0 and residual_signal[left_idx] == 0:
                left_idx -= 1
            while right_idx < len(residual_signal) and residual_signal[right_idx] == 0:
                right_idx += 1

            # Check bounds and interpolate using numpy.interp
            if left_idx >= 0 and right_idx < len(residual_signal):
                residual_signal[idx] = np.interp(idx, [left_idx, right_idx], [residual_signal[left_idx], residual_signal_1[right_idx]])

        residual_with_zeros = np.copy(residual_signal)
        resnon_zero_elements = residual_signal[residual_signal != 0]
        res_mean = np.mean(resnon_zero_elements)
        

        # three signals from 2 sources
        data = np.vstack((ecg_signal, ecg_signal_ch2, ecg_signal_ch3))

        ica = FastICA(n_components=3, whiten="arbitrary-variance", whiten_solver="eigh")
        ica.fit(data.T)

        # Get the independent components
        independent_components = ica.transform(data.T)

        # Separate the main signal and the residual using the independent components
        separated_signal_1 = correct_sign(independent_components[:, 0])[250:3250]
        separated_signal_2 = correct_sign(independent_components[:, 1])[250:3250]
        separated_signal_3 = correct_sign(independent_components[:, 2])[250:3250]

        sample_entropy_1 = ent.sample_entropy(separated_signal_1[:500], 1)
        sample_entropy_2 = ent.sample_entropy(separated_signal_2[:500], 1)
        sample_entropy_3 = ent.sample_entropy(separated_signal_3[:500], 1)

        entropies = [sample_entropy_1, sample_entropy_2, sample_entropy_3]

        max_entropy_index = np.argmax(entropies)
        min_entropy_index = np.argmin(entropies)

        # Perform some function on the signal with maximum entropy
        if max_entropy_index == 0:
            get_hrlis(abs(separated_signal_1), threshold_ratio=0.4, refractory_period=150)

        elif max_entropy_index == 1:
            get_hrlis(abs(separated_signal_2), threshold_ratio=0.4, refractory_period=150)

        else:
            get_hrlis(abs(separated_signal_3), threshold_ratio=0.4, refractory_period=150)

        # Perform some function on the signal with maximum entropy
        if min_entropy_index == 0:
            get_hrlis_mat(abs(separated_signal_1), threshold_ratio=0.4, refractory_period=150)

        elif min_entropy_index == 1:
            get_hrlis_mat(abs(separated_signal_2), threshold_ratio=0.4, refractory_period=150)

        else:
            get_hrlis_mat(abs(separated_signal_3), threshold_ratio=0.4, refractory_period=150)
        '''
            #pr.disable()
            #s = io.StringIO()
            #sortby = SortKey.CUMULATIVE
            #ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
            #ps.print_stats()
            #print(s.getvalue())
        '''
        end_time = time.time()

        print("Execution time: ", end_time-start_time)