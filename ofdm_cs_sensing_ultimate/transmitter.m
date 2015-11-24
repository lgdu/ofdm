function output = transmitter(PSDU,TXVECTOR)
% baseband transmitter
% PSDU: row vector of PHY sublayer service data unit
% TXVECTOR: structure of TXVECTOR parameters
% output: row vector of time-domain signal of a frame

switch TXVECTOR.DATARATE
    case 6
        Ncbps = 48; % coded bits per OFDM symbols
    case 12
        Ncbps = 96;
    otherwise
        error('parameter not supported');
end;

% generate SIGNAL field
signal = signal_generator(TXVECTOR); % SIGNAL field generation
signal_coded = encoder(signal,[],'SIGNAL_FLAG'); % convolutional encoding
signal_intrlv = interleaver(signal_coded,[],'SIGNAL_FLAG'); % interleaving
signal_map = mapper(signal_intrlv,[],'SIGNAL_FLAG'); % mapping

% generate DATA field
data = data_generator(PSDU,TXVECTOR); % DATA field generation & scrambling
data_coded = reshape(encoder(data,TXVECTOR),Ncbps,[]).'; % Ncbps bits a row
data_intrlv = interleaver(data_coded,TXVECTOR); % interleaving
data_map = mapper(data_intrlv,TXVECTOR); % mapping

% modulating SIGNAL and DATA field
sigdat_pilot = pilot_inserter([signal_map;data_map]); % pilot insertion
sigdat_ifft = ofdm_modulator(sigdat_pilot); % inverse Fourier transform
sigdat_gi = gi_inserter(sigdat_ifft); % guard interval insertion

% concatenating short training symbols, long training symbols, SIGNAL
% field, DATA field into a single time-domain signal
len = 320+80*size(sigdat_gi,1)+1; % total length of time-domain signal
output = zeros(1,len);
output = output+[short_generator(),zeros(1,len-161)];
output = output+[zeros(1,160),long_generator(),zeros(1,len-321)];

num = size(sigdat_gi,1); % number of OFDM symbols in SIGNAL and DATA field

for i = 1:num
    output = output+[zeros(1,320+80*(i-1)),sigdat_gi(i,:),...
        zeros(1,80*(num-i))];
end;
