function output = gi_inserter(input)
% guard interval insertion and windowing
% input: matrix of time domain OFDM signal, each row an OFDM symbol
% output: matrix of GI inserted OFDM signal, each row an OFDM symbol

Nsym = size(input,1); % number OFDM symbols
W = [0.5,ones(1,79),0.5]; % GI insertaion and windowing
output = [input(:,49:64),input,input(:,1)].*repmat(W,Nsym,1);
