function show_distance_matrix(D, path)

imagesc(D);
colormap(1-gray)
hold on;
  plot(path(:,2), path(:,1), 'r');
hold off;
