% Adapted from http://www.mathworks.com/help/matlab/ref/fft.html

[y,Fs] = wavread('Swanson_J_mod.wav'); % Wav plus noise
T = 1/Fs;		% Sample time
L = length(y);	% Length of signal
t = (0:L-1)*T;	% Time vector

NFFT = 2^nextpow2(L); % Next power of 2 from length of y
Y = fft(y,NFFT)/L;
f = Fs/2*linspace(0,1,NFFT/2+1);
plot(f,2*abs(Y(1:NFFT/2+1))) 
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')
