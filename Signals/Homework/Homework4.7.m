a=1;
b=[0 1 2 3 4 5];
x1=[1 zeros(1, 9)];
x2=ones(1, 10);
h=filter(b,a,x1);
s=filter(b,a,x2);
n=0:9;

subplot(2,1,1)
stem(n,h)
xlabel('n')
ylabel('h[n]')
title('Impulse Response')
grid

subplot(2,1,2)
stem(n,s)
xlabel('n')
ylabel('s[n]')
title('Unit-step Response')
grid