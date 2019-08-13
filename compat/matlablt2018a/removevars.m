function varargout = removevars(varargin)

% T2 = removevars(T1,vars) deletes the table variables specified
% by vars and copies the remaining variables to T2 (see diagram). You
% can specify variables by name, by position, or using logical indices.
%
% Use as
%   T2 = removevars(T1, vars)
%
% This is a compatibility function that should only be added to the path on
% MATLAB versions prior to R2018a.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% see https://github.com/fieldtrip/fieldtrip/issues/899

alternatives = which(mfilename, '-all');
if ~iscell(alternatives)
  % this is needed for octave, see https://github.com/fieldtrip/fieldtrip/pull/1171
  alternatives = {alternatives};
end

if exist(mfilename, 'builtin') || any(strncmp(alternatives, matlabroot, length(matlabroot)) & cellfun(@isempty, regexp(alternatives, fullfile('private', mfilename))))
  % remove this directory from the path
  p = fileparts(mfilename('fullpath'));
  warning('removing "%s" from your path, see http://bit.ly/2SPPjUS', p);
  rmpath(p);
  % call the original MATLAB function
  if exist(mfilename, 'builtin')
    [varargout{1:nargout}] = builtin(mfilename, varargin{:});
  else
    [varargout{1:nargout}] = feval(mfilename, varargin{:});
  end
  return
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% this is where the actual replacement code starts
% function T2 = removevars(T1, vars)

% deal with the input arguments
if nargin==2
  [T1, vars] = deal(varargin{1:2});
else
  error('incorrect number of input arguments')
end

if islogical(vars)
  vars = T1.Properties.VariableNames(vars);
  T2 = removevars(T1, vars);
elseif isnumeric(vars)
  vars = T1.Properties.VariableNames(vars);
  T2 = removevars(T1, vars);
elseif ischar(vars)
  vars = {vars};
  T2 = removevars(T1, vars);
elseif iscell(vars)
  T2 = table();
  for i=1:numel(T1.Properties.VariableNames)
    var = T1.Properties.VariableNames{i};
    if ~ismember(var, vars)
      T2.(var) = T1.(var);
    end
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% deal with the output arguments

varargout = {T2};
