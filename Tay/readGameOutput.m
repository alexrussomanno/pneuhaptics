% This MATLAB file reads the output from the serial port
% The first column is the button that is pressed [0,1,2,3], second column is the time
% in milliseconds that a cue is triggerred, and the third column is the actual time that user responds to the cue 

fileName = '16_39_11_21_2018.txt';
M = dlmread(fileName);
% Compute number of rows m and columns n
[m,n]=size(M);

%%
% Create variable that stores cueTime and responseTime of a specific button
% Format: [cueTime,responseTime]
position0=M(M(:,1) == 0,2:3);
position1=M(M(:,1) == 1,2:3);
position2=M(M(:,1) == 2,2:3);
position3=M(M(:,1) == 3,2:3);

reaction0=position0(:,2)-position0(:,1);
reaction1=position1(:,2)-position1(:,1);
reaction2=position2(:,2)-position2(:,1);
reaction3=position3(:,2)-position3(:,1);

% Plot cueTime in the x-axis
x0=position0(:,1);
x1=position1(:,1);
x2=position2(:,1);
x3=position3(:,1);

y0=reaction0;
y1=reaction1;
y2=reaction2;
y3=reaction3;

hold on
scatter(x0,y0);
scatter(x1,y1);
scatter(x2,y2);
scatter(x3,y3);




    