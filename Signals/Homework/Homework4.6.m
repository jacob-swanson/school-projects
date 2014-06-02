a=[1 0 -0.2];
b=1;
x=[1 zeros(1, 10)];
y=filter(b,a,x);
n=0:10;
stem(n,y)
xlabel('n')
ylabel('h[n]')
title('Impulse Response')
grid