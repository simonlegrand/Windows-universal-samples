function test_serveur()
port = 5151;
% Add sub-functions to Matlab Search Path
add_function_paths();
defaultoptions=struct('www_folder','www','temp_folder','www/temp','verbose',true,'defaultfile','/index.html','mtime1',10.,'mtime2',10.); 
config=defaultoptions;
TCP=JavaTcpServer('initialize',[],port,config);

while(true)
    % Wait for connections of browsers
    TCP=JavaTcpServer('accept',TCP,[],config);
    [TCP,request_data]=JavaTcpServer('read',TCP,[],config);
    data = request_data';
    sz = data(1,1:2);
    mat(:,:,1) = reshape(data(3, 3:end),sz)';
    mat(:,:,2) = reshape(data(2, 3:end),sz)';
    mat(:,:,3) = reshape(data(1, 3:end),sz)';
    figure, imshow(mat,[])
end

end

function mat = string_to_matrix(str)
    % Transform str into a matrix of int.
    % The string received is under the form
    % 'rowxcol-'x1,x2,x3,...,', where
    % row and col are the image dimensions and
    % x1, x2, x3 are pixel values stores in row-major
    % order.
    i = strfind(str,'-');
    shape = str(1:i-1); data = str(i+1:end-1);
    shape = strsplit(shape,'x');
    row = shape(1); col = shape(2);
    data = strsplit(data,',');
    mat = cellfun(@str2num,data(1:end));
    mat = reshape(mat,str2double(row),str2double(col));
end

function add_function_paths()
try
    functionname='webserver.m';
    functiondir=which(functionname);
    functiondir=functiondir(1:end-length(functionname));
    addpath([functiondir '/functions'])
catch me
    disp(me.message);
end
end