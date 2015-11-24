function output = long_generator()
% long training symbol generator
% output: row vector of time domain representation of long training symbol

L = [1,1,-1,-1,1,1,-1,1,-1,1,1,1,1,1,1,-1,-1,1,1,-1,1,-1,1,1,1,1,... 
    0,1,-1,-1,1,1,-1,1,-1,1,-1,-1,-1,-1,-1,1,1,-1,-1,1,-1,1,-1,1,1,1,1];
W = [0.5,ones(1,159),0.5];

symbol = ifft([L(27:53),zeros(1,11),L(1:26)]);
output = [symbol(33:64),symbol,symbol,symbol(1)].*W;
