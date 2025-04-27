within ExternalLibrary;
function externalLibraryFunction
  input String filename "Path of the data file" annotation(Dialog(loadSelector(filter="Data File (*.txt)", caption="Select the data file")));
  input Real[2] u={1,2} "Input";
  input ExternalLibrary.PythonParams pythonParams = PythonParams() "Python parameters";
  output Real[2] y "Output";
protected
  String errorMessage;
algorithm
  (y, errorMessage) := ExternalLibrary.Internal.externalLibraryFunction(
    filename,
    size(u, 1),
    u,
    size(y, 1),
    pythonParams);

  assert(Modelica.Utilities.Strings.isEmpty(errorMessage), errorMessage);
end externalLibraryFunction;
