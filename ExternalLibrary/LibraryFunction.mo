within ExternalLibrary;
function libraryFunction
  input String filename=Modelica.Utilities.Files.loadResource("modelica://ExternalLibrary/Resources/Data/data.txt");
  input String pythonHome=Modelica.Utilities.System.getEnvironmentVariable("PYTHONHOME");
  input Real[2] u={1,2};
  output Real[2] y;
protected
  String errorMessage;
algorithm
  (y,errorMessage) := ExternalLibrary.Internal.externalLibraryFunction(
    filename,
    pythonHome,
    u);

  assert(Modelica.Utilities.Strings.isEmpty(errorMessage), errorMessage);
end libraryFunction;
