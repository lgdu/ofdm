function output = dec2bit(input,width)
% convert decimal integer to binary bits
% input: decimal integer
% width: width of binary bits
% output: row vector of binary bits

string = dec2bin(input);
len = length(string);
binary = zeros(1,len);
binary(find(string == '1')) = 1;

if nargin == 1
    output = binary;
else
    if width > len
        output = [zeros(1,width-len),binary];
    else
        output = binary;
    end
end;
