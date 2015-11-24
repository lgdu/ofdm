function output = pilot_inserter(input)
% pilot insertion
% input: matrix of frequency domain OFDM signal without pilot, each row an
% OFDM symbol
% output: matrix of frequency domain OFDM signal with pilot inserted, each
% row an OFDM symbol

Nsym = size(input,1); % number of OFDM symbols
pilot = repmat([1,1,1,-1],Nsym,1).*...
    repmat(-2*scrambler(Nsym,[1,1,1,1,1,1,1]).'+1,1,4);
output = [input(:,1:5),pilot(:,1),input(:,6:18),pilot(:,2),...
    input(:,19:30),pilot(:,3),input(:,31:43),pilot(:,4),input(:,44:48)];
