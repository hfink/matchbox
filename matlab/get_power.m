function [ power ] = get_power( spectrum )
% rows are spectrum bins, cols are frames
% returns the normalized power distribution

[rows, cols] = size(spectrum);

max_power = 0;

rows_used = rows - 100; 

power = zeros(1, cols);
for col=1:cols
    for row=1:rows_used
        bin_power = spectrum(row, col)^2;
        power(1,col) = power(1,col) + bin_power;
    end
    max_power = max(max_power, power(1, col));
end

power = power * 1/max_power;

