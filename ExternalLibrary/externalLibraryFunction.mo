within ExternalLibrary;
function externalLibraryFunction
  input String filename;
  input String moduleName;
  input String functionName;
  input String pythonHome;
  input Real[2] u={1,2};
  output Real[2] y;
protected
  String errorMessage;
algorithm
  (y,errorMessage) := ExternalLibrary.Internal.externalLibraryFunction(
    filename,
    moduleName,
    functionName,
    pythonHome,
    size(u, 1),
    u,
    size(y, 1));

  assert(Modelica.Utilities.Strings.isEmpty(errorMessage), errorMessage);
end externalLibraryFunction;
