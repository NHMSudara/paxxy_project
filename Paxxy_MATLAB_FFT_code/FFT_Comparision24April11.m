% Read the data from the CSV file
data3B = readtable('ADS_Data_12.csv');
data5C2 = readtable('ADS_Data_13.csv');

M11 = table2array(data3B(:, 1));
M12 = table2array(data3B(:, 2));
M13 = table2array(data3B(:, 3));
M14 = table2array(data3B(:, 4));

M21 = table2array(data5C2(:, 1));
M22 = table2array(data5C2(:, 2));
M23 = table2array(data5C2(:, 3));
M24 = table2array(data5C2(:, 4));

sampleStart = 2000;
sampleCount = sampleStart + 20000;

M11 = M11(sampleStart:sampleCount);
M12 = M12(sampleStart:sampleCount);
M13 = M13(sampleStart:sampleCount);
M14 = M14(sampleStart:sampleCount);
M21 = M21(sampleStart:sampleCount);
M22 = M22(sampleStart:sampleCount);
M23 = M23(sampleStart:sampleCount);
M24 = M24(sampleStart:sampleCount);

L1 = length(M11); % Length of the ECG data
L2 = length(M21); % Length of the ECG data
%remove large outliers
for i = 1:L2-2
    if abs(M21(i)-M21(i+1))>10000
        M21(i+1) = M21(i);
    end
    if abs(M22(i)-M22(i+1))>10000
        M22(i+1) = M22(i);
    end
    if abs(M23(i)-M23(i+1))>10000
        M23(i+1) = M23(i);
    end
    if abs(M24(i)-M24(i+1))>10000
        M24(i+1) = M24(i);
    end
end

% Standardize M1
M11_mean = mean(M11(:));
M12_mean = mean(M12(:));
M13_mean = mean(M13(:));
M14_mean = mean(M14(:));

M11_std = std(M11(:));
M12_std = std(M12(:));
M13_std = std(M13(:));
M14_std = std(M14(:));

M11 = (M11 - M11_mean) / M11_std;
M12 = (M12 - M12_mean) / M12_std;
M13 = (M13 - M13_mean) / M13_std;
M14 = (M14 - M14_mean) / M14_std;

% Standardize M2
M21_mean = mean(M21(:));
M22_mean = mean(M22(:));
M23_mean = mean(M23(:));
M24_mean = mean(M24(:));

M21_std = std(M21(:));
M22_std = std(M22(:));
M23_std = std(M23(:));
M24_std = std(M24(:));

M21 = (M21 - M21_mean) / M21_std;
M22 = (M22 - M22_mean) / M22_std;
M23 = (M23 - M23_mean) / M23_std;
M24 = (M24 - M24_mean) / M24_std;


figure;
subplot(4, 2, 1); plot(M11); %xlim([2 100]); ylim([0 0.000001]); grid on;
subplot(4, 2, 3); plot(M12) ; %xlim([2 100]); ylim([0 0.000001]);grid on;
subplot(4, 2, 5); plot(M13) ; %xlim([2 100]); ylim([0 0.000001]);grid on;
subplot(4, 2, 7); plot(M14) ; %xlim([2 100]); ylim([0 0.000001]);grid on;



subplot(4, 2, 2); plot(M21); %xlim([2 100]); ylim([0 1500]); grid on;
subplot(4, 2, 4); plot(M22) ; %xlim([2 100]); ylim([0 1500]);grid on;
subplot(4, 2, 6); plot(M23) ; %xlim([2 100]); ylim([0 1500]);grid on;
subplot(4, 2, 8); plot(M24) ; %xlim([2 100]); ylim([0 1500]);grid on;

% Sample Rate and Data Length
Fs = 500; % Change this to your actual sampling rate

% Create a frequency vector
f1 = Fs*(0:(L1/2))/L1;

% Compute the FFT
Y11 = fft(M11);
Y12 = fft(M12);
Y13 = fft(M13);
Y14 = fft(M14);

% Create a frequency vector
f2 = Fs*(0:(L2/2))/L2;
% Compute the FFT
Y21 = fft(M21);
Y22 = fft(M22);
Y23 = fft(M23);
Y24 = fft(M24);


% Compute the two-sided spectrum and then the single-sided spectrum
P2_11 = abs(Y11/L1);
P1_11 = P2_11(1:L1/2+1);% Only need to take half of the FFT output, 
P1_11(2:end-1) = 2*P1_11(2:end-1); % multiply by 2 for proper scaling

P2_12 = abs(Y12/L1);
P1_12 = P2_12(1:L1/2+1);
P1_12(2:end-1) = 2*P1_12(2:end-1); % Only need to take half of the FFT output, and multiply by 2 for proper scaling

P2_13 = abs(Y13/L1);
P1_13 = P2_13(1:L1/2+1);
P1_13(2:end-1) = 2*P1_13(2:end-1); % Only need to take half of the FFT output, and multiply by 2 for proper scaling

P2_14 = abs(Y14/L1);
P1_14 = P2_14(1:L1/2+1);
P1_14(2:end-1) = 2*P1_14(2:end-1); % Only need to take half of the FFT output, and multiply by 2 for proper scaling
%__________________________________
P2_21 = abs(Y21/L2);
P1_21 = P2_21(1:L2/2+1);
P1_21(2:end-1) = 2*P1_21(2:end-1); % Only need to take half of the FFT output, and multiply by 2 for proper scaling

P2_22 = abs(Y22/L2);
P1_22 = P2_22(1:L2/2+1);
P1_22(2:end-1) = 2*P1_22(2:end-1); % Only need to take half of the FFT output, and multiply by 2 for proper scaling

P2_23 = abs(Y23/L2);
P1_23 = P2_23(1:L2/2+1);
P1_23(2:end-1) = 2*P1_23(2:end-1); % Only need to take half of the FFT output, and multiply by 2 for proper scaling

P2_24 = abs(Y24/L2);
P1_24 = P2_24(1:L2/2+1);
P1_24(2:end-1) = 2*P1_24(2:end-1); % Only need to take half of the FFT output, and multiply by 2 for proper scaling

% figure;
% plot(f2, P1_22); xlim([2 100]); ylim([0 1000]); grid on;

% Plotting the single-sided amplitude spectrum
figure;
subplot(4, 2, 1); plot(f1, P1_11); xlim([0 100]); ylim([0 0.3]); grid on;
subplot(4, 2, 3); plot(f1, P1_12) ; xlim([0 100]); ylim([0 0.3]);grid on;
subplot(4, 2, 5); plot(f1, P1_13) ; xlim([0 100]); ylim([0 0.3]);grid on;
subplot(4, 2, 7); plot(f1, P1_14) ; xlim([0 100]); ylim([0 0.3]);grid on;

subplot(4, 2, 2); plot(f2, P1_21); xlim([0 100]); ylim([0 0.3]); grid on;
subplot(4, 2, 4); plot(f2, P1_22) ; xlim([0 100]); ylim([0 0.3]);grid on;
subplot(4, 2, 6); plot(f2, P1_23) ; xlim([0 100]); ylim([0 0.3]);grid on;
subplot(4, 2, 8); plot(f2, P1_24) ; xlim([0 100]); ylim([0 0.5]);grid on;
%title('Single-Sided Amplitude Spectrum of ECG')
%xlabel('Frequency (Hz)')
%ylabel('|P1(f)|')
