function output = ofdm_modulator(input)
% inverse Fourier transform
% input: matrix of frequency domain OFDM signal, each row an OFDM symbol
% output: matrix of time domain OFDM signal, each row an OFDM symbol

Nsym = size(input,1);
output = ifft([zeros(Nsym,1),input(:,27:52),...
    zeros(Nsym,11),input(:,1:26)],[],2);
