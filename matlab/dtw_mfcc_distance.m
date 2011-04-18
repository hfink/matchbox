function minimum_distance = dtw_mfcc_distance(wav_file1, wav_file2, ~)
% Load wave files and return minimal time normalized dtw mfcc distance.

% Matlab doesn't have real default arguments, use an ugly workaround
if (nargin<3)
    debug=false;
else
    debug=true;
end

% get data and sampling rate from wav_file1 and wav_file2
[data1, sr1] = wavread(wav_file1);
[data2, sr2] = wavread(wav_file2);

assert(sr1==sr2, 'sampling rates of two compared files should be equal');

if (debug)
    % play samples in stereo if we are in debug mode
    len = max(length(data1), length(data2));
    pad1 = len - length(data1);
    pad2 = len - length(data2);
    soundsc([padarray(data1, pad1, 0, 'post'), ...
             padarray(data2, pad2, 0, 'post')],...
            sr1);
end

% get mfcc ceps feature vectors
[ceps1,freqresp1,fb1,fbrecon1,freqrecon1] = mfcc(data1, sr1, 100);
[ceps2,freqresp2,fb2,fbrecon2,freqrecon2] = mfcc(data2, sr2, 100);

% energy1 = get_power(freqresp1);
% energy2 = get_power(freqresp2);

% discard the first row of the feature vector
features1 = ceps1(2:8,:);
features2 = ceps2(2:8,:);

% TODO: find a usable adjustment window size
adjustment_window_size=20;

[minimum_distance, d, g, path] = dtw(features1, ...
                                     features2, ...
                                     adjustment_window_size);

if (debug)
    % show local distance matrix
    subplot(321);
    show_distance_matrix(d, path);

    % show global distance matrix
    subplot(322);
    show_distance_matrix(g, path);

    % show feature 1 spectrogram
    subplot(323);
    show_mfcc_data(freqresp1);
    
    % show feature 2 spectrogram
    subplot(324);
    show_mfcc_data(freqresp2);        
    
    % show feature 1 data
    subplot(325);
    show_mfcc_data(features1);
    
    % show feature 2 data
    subplot(326);
    show_mfcc_data(features2);    
    
end
