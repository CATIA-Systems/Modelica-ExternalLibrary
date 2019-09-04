within ExternalLibrary;
model ExternalLibraryFunctionExample
  extends Modelica.Icons.Example;

  parameter Real y[2]=ExternalLibrary.externalLibraryFunction(
      Modelica.Utilities.Files.loadResource("modelica://ExternalLibrary/Resources/Data/data.txt"),
      "external_library",
      "external_library_function",
      Modelica.Utilities.System.getEnvironmentVariable("PYTHONHOME"),
      {1,2});

end ExternalLibraryFunctionExample;
