within ExternalLibrary;
record PythonParams

  String pythonDllPath = "" "Path of the Python DLL" annotation(Dialog(loadSelector(filter="Python DLL (*.dll)", caption="Select the Python DLL")));
  String pythonExePath = "" "Path of the Python executable" annotation(Dialog(loadSelector(filter="Python Executable (*.exe)", caption="Select the Python Executable")));
  String moduleName = "" "Name of the Python module";
  String functionName = "" "Name of the Python function";

  annotation (Icon(coordinateSystem(preserveAspectRatio=false)), Diagram(
        coordinateSystem(preserveAspectRatio=false)));
end PythonParams;
