function output = inverse(input)
% inverse the order of the elements in a vector
% input: row vector of elements whose order is to be inversed
% output: row vector of elements whose order is inversed

len = length(input);
output = zeros(1,len);

for i = 1:len;
    output(len-i+1) = input(i);
end;
