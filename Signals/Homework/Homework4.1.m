den=[1 3 (pi^2 + 3) (pi^2 + 1)];
num1=[1 -3 (pi^2 + 3) (-(pi^2) - 1)];
num2=[0 1 0 pi^2];
num3=[0 0 1 -1];
[h1,w1] = freqs(num1,den,100);
[h2,w2] = freqs(num2,den,100);
[h3,w3] = freqs(num3,den,100);

subplot(3,1,1)
plot(w1,abs(h1))
title('Magnitude Response: H_1(s)')
xlabel('\Omega')
ylabel('|H(j\Omega)|')
grid

subplot(3,1,2)
plot(w2,abs(h2))
title('Magnitude Response: H_2(s)')
xlabel('\Omega')
ylabel('|H(j\Omega)|')
grid

subplot(3,1,3)
plot(w3,abs(h3))
title('Magnitude Response: H_3(s)')
xlabel('\Omega')
ylabel('|H(j\Omega)|')
grid
