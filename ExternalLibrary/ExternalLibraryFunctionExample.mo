within ExternalLibrary;
model ExternalLibraryFunctionExample
  extends Modelica.Icons.Example;

  parameter Real y[2]=ExternalLibrary.externalLibraryFunction(
      Modelica.Utilities.Files.loadResource("modelica://ExternalLibrary/Resources/Data/data.txt"),
    {1,2},
    ExternalLibrary.PythonParams(moduleName="external_library", functionName="external_library_function"));

end ExternalLibraryFunctionExample;
