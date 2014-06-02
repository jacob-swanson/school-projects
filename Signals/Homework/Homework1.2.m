Ts1=0:0.1:3;
Ts2=0:0.5:3;
Ts3=0:1:3;
Ts4=0:1/1000:3;

y1=2*cos(2*pi*1*Ts1);
y2=2*cos(2*pi*1*Ts2);
y3=2*cos(2*pi*1*Ts3);
y4=2*cos(2*pi*1*Ts4);

subplot(2,2,1);
stem(Ts1,y1);
xlabel('Time (s)');
title('Ts=0.1');

subplot(2,2,2);
stem(Ts2,y2)
xlabel('Time (s)');
title('Ts=0.5');

subplot(2,2,3);
stem(Ts3,y3)
xlabel('Time (s)');
title('Ts=1');

subplot(2,2,4);
plot(Ts4,y4)
xlabel('Time (s)');
title('Ts=0.001');