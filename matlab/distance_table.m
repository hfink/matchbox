function [minimum_cost_table, ...
          recall, ...
          precision,...
          specifity,...
          fmeasure,... 
          suggested_treshold] = distance_table(n_speakers, n_samples, threshold)
%distance_table Runs dtw with our features on n speakers and n samples.
% We also calculates the recall and precision values for a given threshold
% Also
% returning the F_1-measure which combines these two values. A F-measure
% of 0.5 would be optimal.
%
% Precision : true_positives / (true_positives + false_positives)
% Recall : true_positives / (true_positives + false_negatives)
% Specifity : true_negatives / (true_negatives + false_positives)
% F-measure : 2*precision*recall / (precision + recall)
% suggested_threshold : stores a minimum threshold with the current
% configuration that would be required to reach a recall of 100% (does not
% consider false positives.)

minimum_cost_table=zeros(n_samples, n_samples, n_speakers, n_speakers);
minimum_cost_table(:,:,:,:) = NaN;

true_positives = 0;
true_negatives = 0;
false_negatives = 0;
false_positives = 0;
suggested_treshold = 0;

for x=1:n_speakers
    for y=1:n_speakers
        for i=1:n_samples
            for j=1:n_samples
                if (i==j && x==y)
                    % don't calculate D(A,B) for same speaker & same sample
                    distance = 0;
                elseif (i<=j && x<=y)
                    % calculate minimum dtw mfcc distance
                    sample1 = sprintf('samples/0%d-%d.wav', i, x);
                    sample2 = sprintf('samples/0%d-%d.wav', j, y);
                    distance = dtw_mfcc_distance(sample1, sample2);
                    match = distance < threshold;
                    
                    if ( i==j )
                        % Means the two samples represent the same
                        % utterance
                        suggested_treshold = max(suggested_treshold,...
                                                 distance);
                        
                        if ( match )
                            true_positives = true_positives + 1;
                        else
                            false_negatives = false_negatives + 1;
                        end                        
                    else
                        if ( match )
                            false_positives = false_positives + 1;
                        else
                            true_negatives = true_negatives + 1;
                        end                                                
                    end
                                 
                else
                    distance = NaN;
                end
                minimum_cost_table(i, j, x, y) = distance;
            end
        end
    end
end

recall = true_positives / (true_positives + false_negatives);
precision = true_positives / (true_positives + false_positives);
specifity = true_negatives / (true_negatives + false_positives);
fmeasure = 2*precision*recall / (precision + recall);

