function [data,header] = DAQfileReadArduino(filename,OutputType,len)
% function to read data files produced by NIDAQMX app
% inputs:
%   filename    name of file to be read in
%   OutputType  type method for data to be output. Options include 'table'
%               'cell', and 'matrix' (default)
%
% outputs:
%   data        matrix with timestamps and data collected by DAQ during session
%   header      variable containing information on the start time of the
%               session, number of channels involved (including the time
%               channel), and the channel names
%   len         specifies the number of time samples to be read, default=inf
if nargin<3 || isempty(len)
    len = inf;
end
if nargin<2
    OutputType = 'matrix';
else
    OutputType = lower(OutputType);
end
if nargin<1 || isempty(filename)
    [file, path] = uigetfile('.bin','Select file to be read');
    if ischar(path)
        filename = [path file];
    else
        return;
    end
end

FID = fopen(filename,'r');
header.FileName = filename;
header.TimeStamp = datetime(fread(FID,1,'double'),'ConvertFrom','datenum');
header.SampRate = fread(FID,1,'double');
header.ChannelNumber = fread(FID,1,'uint8');
header.Names = cell(header.ChannelNumber,1);
for n=1:header.ChannelNumber
    charLength = fread(FID,1,'uint8');
    name = fread(FID,charLength,'*char');
    header.Names{n} = name(:)';
end
data = fread(FID,[header.ChannelNumber len],'uint16');
fclose(FID);

data = data';
if any(data(:,2)>=2^15)
    header.Names{end+1} = 'Trigger';
    trigger = data(:,2)>=2^15;
    data(:,3) = trigger*5;
end
switch(OutputType)
    case 'table'
        data = array2table(data,'VariableNames',header.Names);
    case 'cell'
        data2 = cell(1,size(data,2));
        for n=1:size(data,2)
            data2{n} = data(:,n);
        end
        data = data2;
end
