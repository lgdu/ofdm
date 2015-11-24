function output = data_generator(PSDU,TXVECTOR)
% DATA field generator
% PSDU: row vector of PHY sublayer service data unit
% TXVECTOR: structure of TXVECTOR parameters
% output: row vector of scrambled DATA field

switch TXVECTOR.DATARATE
    case 6
        Ndbps = 24; % data bits per OFDM symbol
    case 12
        Ndbps = 48;
    otherwise
        error('parameter not supported');
end;

Nsym = ceil((16+8*TXVECTOR.LENGTH+6)/Ndbps); % number of OFDM symbols
Ndata = Nsym*Ndbps; % number of bits in the DATA field

data = zeros(1,Ndata);
data(17:16+8*TXVECTOR.LENGTH) = PSDU;

output = rem(data+scrambler(Ndata,[1,0,1,1,1,0,1]),2); % scrambling
output(17+8*TXVECTOR.LENGTH:22+8*TXVECTOR.LENGTH) = zeros(1,6); % TAIL
