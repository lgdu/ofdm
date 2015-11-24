function output = encoder(input,TXVECTOR,SIGNAL_FLAG)
% convolutional encoder
% input: row vector of SIGNAL field or DATA field
% TXVECTOR: structure of TXVECTOR parameters
% SIGNAL_FLAG: indicator of SIGNAL field
% output: row vector of encoded SIGNAL field or DATA field

if nargin == 2 % encoding DATA field
    rate = TXVECTOR.DATARATE;
else % encoding SIGNAL field
    rate = 6;
end;

trellis = poly2trellis(7,[133,171]); % trellis structure

switch rate
    case 6
        output = convenc(input,trellis); % convolutional encoding
    case 12
        %%puncpat = [1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1]; % puncture pattern
        %%output = convenc(input,trellis,puncpat); % convolutional encoding
        output = convenc(input,trellis); % convolutional encoding
    otherwise
        error('parameter not supported');
end;
