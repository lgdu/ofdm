function output = mapper(input,TXVECTOR,SIGNAL_FLAG)
% subcarrier mapper
% input: matrix of data bits to be mapped into constellation
% TXVECTOR: structure of TXVECTOR parameters
% SIGNAL_FLAG: indicator of SIGNAL field
% output: matrix of data bits mapped into constellation

if nargin == 2 % mapping DATA field
    rate = TXVECTOR.DATARATE;
else % mapping SIGNAL field
    rate = 6;
end;

switch rate
    case 6 % BPSK
        output = 2*input-1;
    case 12 % QPSK
        [m,n] = size(input);
        output = zeros(m,n/2);
        output = 2*input(:,1:2:n-1)-1;
        output = output+ j*2*input(:,2:2:n)-j;
        output = output*sqrt(1/2);
%    case 36 % 16QAM, Gray-coded
%        output = zeros(size(input)./[1,4]);
%        
%        for m = 1:size(input,1)
%            symbol = reshape(input(m,:),4,[]);
%        
%            for n = 1:size(symbol,2)
%                col = 2*symbol(1,n)+symbol(2,n);
%                lin = 2*symbol(3,n)+symbol(4,n);
%                
%                switch col
%                    case 0
%                        output(m,n)=-3;
%                    case 1
%                        output(m,n)=-1;
%                    case 3
%                        output(m,n)=1;
%                    case 2
%                        output(m,n)=3;
%                end;
%                
%                switch lin
%                    case 0
%                        output(m,n)=output(m,n)-3*j;
%                    case 1
%                        output(m,n)=output(m,n)-j;
%                    case 3
%                        output(m,n)=output(m,n)+j;
%                    case 2
%                        output(m,n)=output(m,n)+3*j;
%                end;
%            end
%        end;
%        
%        output = sqrt(1/10)*output; % normalization
    otherwise
        error('parameter not supported');
end;
