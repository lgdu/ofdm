function output = interleaver(input,TXVECTOR,SIGNAL_FLAG)
% data interleaver
% input: matrix of data bits to be interleaved, each row a block
% TXVECTOR: structure of TXVECTOR parameters
% SIGNAL_FLAG: indicator of SIGNAL field
% output: matrix of interleaved data bits, each row a block

if nargin == 2 % interleaving DATA field
    rate = TXVECTOR.DATARATE;
else % interleaving SIGNAL field
    rate = 6;
end;

switch rate
    case 6
        Nbpsc = 1; % coded bits per subcarrier
        Ncbps = 48; % coded bits per OFDM symbol
    case 12
        Nbpsc = 2;
        Ncbps = 96;
    otherwise
        error('parameter not supported');
end;

s = max(Nbpsc/2,1);
j = [0:Ncbps-1];
i = s*floor(j/s)+mod((j+floor(16*j/Ncbps)),s);
k = 16*i-(Ncbps-1)*floor(16*i/Ncbps);

output = intrlv(input.',k+1).'; % interleaving
