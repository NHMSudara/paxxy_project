%%%%%%%%%%%%%%%%%  GENERAL SETTINGS %%%%%%%%%%%%%%%
    N = 120000;       % number of samples in the data set
    N0 = 2000;         % starting point of the variable obtained from dataset
    cutOffFreqLPF = 160;  % Low pass filter cut off frequency (in Hz)
    cutOffFreqHPF = 0.5;    % High pass filter cut off frequency (in Hz)
    powerLineFreq = 50;     % power line freq. in Hz (60 for US)
    SamplingFreq = 500;     % sampling frequency of our data
    runningAvSampleSize = 100;


%_DATA AQUICISION
%t=xlsread('test1_10.50AM.csv','A1:A100000'); %time column in ms
y1=xlsread('ADS_Data_67.xlsx','A1:A200000'); % ra signal
y2=xlsread('ADS_Data_67.xlsx','B1:B200000'); % ll signal
y3=xlsread('ADS_Data_67.xlsx','C1:C200000'); % la signal
y4=xlsread('ADS_Data_67.xlsx','D1:D200000'); % v1 signal

t = (1:1:N);    %time variable (X axis)
%Define four variables from pre-copied data
%    y1 = M2F1;
%    y2 = M2F2;
%    y3 = M2F3;
%    y4 = M2F4;

%     %cut off any outliers which are way high in amplitude
%     maxAllowed = 10000;
%     for n1 = 1:N
%         if y1(n1)>maxAllowed || y1(n1)<-maxAllowed
%             y1(n1) = 0;
%         end
%         if y2(n1)>maxAllowed || y2(n1)<-maxAllowed
%             y2(n1) = 0;
%         end
%         if y3(n1)>maxAllowed || y3(n1)<-maxAllowed
%             y3(n1) = 0;
%         end
%         if y4(n1)>maxAllowed || y4(n1)<-maxAllowed
%             y4(n1) = 0;
%         end
%     end

%     y1 = M2F1(N0:1:N+N0-1);
%     y2 = M2F2(N0:1:N+N0-1);
%     y3 = M2F3(N0:1:N+N0-1);
%     y4 = M2F4(N0:1:N+N0-1);
%_END OF DATA AQUICISION________________


%=================  DATA FILTERING  ===================
    % LPF
        %...................................................
        %this is the original settings from Prof. Rohan
        f = [0.12 0.16]; % Cutoff frequencies
        a = [1 0];  % Desired amplitudes
        rp = 0.01;  % Passband ripple in dB
        rs = 80;    % Stopband ripple in dB
        %this has a much clear ECG signal
        f = [0.06 0.09]; a = [1 0]; rp = 0.01; rs = 80;
        %...................................................

            w = (-1+(1/N)):2/N:(1-(1/N));
            %[n,fo,ao,w] = firpmord(f,a,dev) returns the approximate order n,
                %normalized frequency band edges fo, frequency band amplitudes ao,
                %and weights w that meet input specifications f, a, and dev.
            dev = [(10^(rp/20)-1)/(10^(rp/20)+1)  10^(-rs/20)];
            [n,fo,ao,w] = firpmord(f,a,dev);

            % Filter (linear-phase FIR filter using the Parks-McClellan algorithm)
            %b = firpm(n,f,a) returns row vector b containing the n+1 coefficients
                %of the order n FIR filter whose frequency-amplitude characteristics
                %match those given by vectors f and a.
            LP = firpm(n,fo,ao,w);

    %HPF settings
        f = [0.004 0.02]; %frequency band for high pass filter
        a = [0 1];
        rp = 0.05;
        rs = 40;
        dev = [(10^(rp/20)-1)/(10^(rp/20)+1)  10^(-rs/20)];
        [n,fo,ao,w] = firpmord(f,a,dev);
        HP = firpm(n,fo,ao,w); % Filter

        % LPF Filtering
        a1 = filter(LP,1,y1); a2 = filter(LP,1,y2);
        a3 = filter(LP,1,y3); a4 = filter(LP,1,y4);
        % HPF Filtering
        z1 = filter(HP,1,a1); z2 = filter(HP,1,a2);
        z3 = filter(HP,1,a3); z4 = filter(HP,1,a4);


% %_______________ CALCULATE MOTHER'S HEART RATE _______________
% % Pan-Tompkins QRS detection algorithm
% [,qrsI,] = pan_tompkin(z3, SamplingFreq, false);
% % Calculate RR intervals (time between QRS complexes)
% rrIntervals = diff(qrsI) / SamplingFreq;  % Convert to seconds
% % Calculate heart rate
% heartRate = 60 ./ rrIntervals;  % Heart rate in beats per minute (BPM)
% % Average heart rate
% averageHeartRate = mean(heartRate);
% % Display average heart rate
% disp(['Average Heart Rate: ', num2str(averageHeartRate), ' BPM']);
% %_____________________________________________________________
%
%     figure;
%     plot (z1);
%
% %1. LOW PASS FILTER
%     % Normalize the frequency with respect to the Nyquist frequency
%     Wn = cutOffFreqLPF / (SamplingFreq / 2);
%     % Create a low-pass Butterworth filter (order 2 for example)
%     [b, a] = butter(2, Wn, 'low');
%     % Apply the filter to the ECG data
%     a1 = filter(b, a, y1);
%     a2 = filter(b, a, y2);
%     a3 = filter(b, a, y3);
%     a4 = filter(b, a, y4);
%
% %2. HIGH PASS FILTER
%     % Normalize the frequency with respect to the Nyquist frequency
%     Wn = cutOffFreqHPF / (SamplingFreq / 2);
%     % Create a high-pass Butterworth filter (order 2 for example)
%     [b, a] = butter(2, Wn, 'high');
%     % Apply the filter to the ECG data
%     z1 = filter(b, a, a1);
%     z2 = filter(b, a, a2);
%     z3 = filter(b, a, a3);
%     z4 = filter(b, a, a4);

%3. Notch filter to remove power line noise (50Hz or 60Hz)
    % Design a notch filter to remove 50Hz power line noise
    wo = powerLineFreq /(SamplingFreq/2);  % 50Hz frequency normalized by Nyquist frequency
    bw = wo/0.5;      % Bandwidth for the notch filter. 35 IS THE ORIGINAL VALUE
    [bn, an] = iirnotch(wo, bw);  % Notch filter design
    % Apply the notch filter to your preprocessed data
    S1 = filter(bn, an, z1);
    S2 = filter(bn, an, z2);
    S3 = filter(bn, an, z3);
    S4 = filter(bn, an, z4);
    %crop off initial high peaks coming from filtering
    S1 = S1(500:end-1);
    S2 = S2(500:end-1);
    S3 = S3(500:end-1);
    S4 = S4(500:end-1);



     figure;
     subplot (2,2,1), plot (y1(500:N)), title('S1');
     subplot (2,2,2), plot (y2(500:N)), title('S2');
     subplot (2,2,3), plot (y3(500:N)), title('S3');
     subplot (2,2,4), plot (y4(500:N)), title('S4');

     figure;
     subplot (2,2,1), plot (S1(500:N)), title('S1');
     subplot (2,2,2), plot (S2(500:N)), title('S2');
     subplot (2,2,3), plot (S3(500:N)), title('S3');
     subplot (2,2,4), plot (S4(500:N)), title('S4');

%filtered data to csv
%data1 = S1(500:N);
%data2 = S2(500:N);
%data3 = S3(500:N);
%data4 = S4(500:N);

%filtered_data = [data1', data2', data3', data4'];
%filtered_data = array2table(filtered_data);
%output_file = 'filtered_data.xlsx';

%writetable(filtered_data, output_file, 'Sheet', 'FilteredData');

%data = [data1, data2, data3];
%filename = 'filtered.csv';
%csvwrite(filename, data);