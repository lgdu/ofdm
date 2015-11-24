function output = signal_generator(TXVECTOR)
% signal field generator
% TXVECTOR: structure of TXVECTOR parameters
% output: row vector of SIGNAL field

output = zeros(1,24);

switch TXVECTOR.DATARATE % RATE field
    case 6
        output(1:4) = [1,1,0,1];
    case 12
        output(1:4) = [0,1,0,1];
    otherwise
        error('parameter not supported.');
end;

output(6:17) = inverse(dec2bit(TXVECTOR.LENGTH,12)); % LENTH field
