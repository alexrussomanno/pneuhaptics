function [M,gameMode,gameInfo,...
    cycle,previewTime,minWaitTime,maxWaitTime,cueTime,...
    pressTime,reactionTime,aveReaction,standDev] = readTxtFile(fileName)
%This function takes in the fileName and organizes the cueTime and
%pressTime into a matrix M

% Read game settings
gameInfo=dlmread(fileName,'\t',[0 1 4 1]);
gameMode=gameInfo(1);
cycle=gameInfo(2);
previewTime=3.*gameInfo(3);
minWaitTime=gameInfo(4);
maxWaitTime=gameInfo(5);

% Place rest of result into matrix
M = dlmread(fileName,'\t',8,0);
% Compute number of rows m and columns n
cueTime=M(:,2);
pressTime=M(:,3);
reactionTime = pressTime-cueTime;
aveReaction = mean(reactionTime(reactionTime>=0));
standDev=std(reactionTime(reactionTime>=0));

end

