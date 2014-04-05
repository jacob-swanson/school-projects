[y,Fs] = wavread('Swanson_J_mod.wav');
length = length(y);

% Take transform
Y = fft(y);
Y = Y(1:length/2+1);
freq = 0:Fs/length:Fs/2;
plot(freq,Y);
xlabel('Hz');