function output = short_generator()
% short training symbol generator
% output: row vector of time domain representation of short training symbol

S = sqrt(13/6) * ...
    [0,0,1+j,0,0,0,-1-j,0,0,0,1+j,0,0,0,-1-j,0,0,0,-1-j,0,0,0,1+j,0,0,0,...
    0,0,0,0,-1-j,0,0,0,-1-j,0,0,0,1+j,0,0,0,1+j,0,0,0,1+j,0,0,0,1+j,0,0];
W = [0.5,ones(1,159),0.5];

symbol = ifft([S(27:53),zeros(1,11),S(1:26)]);
output = [symbol,symbol,symbol(1:33)].*W;
