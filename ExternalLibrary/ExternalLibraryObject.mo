within ExternalLibrary;
model ExternalLibraryObject
  extends Modelica.Blocks.Interfaces.MIMO;

  parameter String filename "Filename of the data file";
  parameter String moduleName "Python module name of the external object";
  parameter String className "Class name of the external object";
  parameter String pythonHome "Path to the Python environment";

  ExternalLibrary.Internal.ExternalLibraryObject externalObject=
      ExternalLibrary.Internal.ExternalLibraryObject(filename, moduleName, className, pythonHome);

protected
  function evaluate
    input ExternalLibrary.Internal.ExternalLibraryObject externalObject;
    input Integer nin;
    input Real[:] u;
    input Integer nout;
    output Real[nout] y;
  external"C" evaluateExternalObject(
        externalObject,
        nin,
        u,
        nout,
        y) annotation (Library="ExternalLibrary");
  end evaluate;

equation
  y = evaluate(externalObject, nin, u, nout);

  annotation (                              experiment(
      StopTime=10,
      __Dymola_fixedstepsize=0.05,
      __Dymola_Algorithm="Euler"), Icon(graphics={Polygon(
          points={{0,40},{0,34},{-54,34},{-54,-64},{44,-64},{44,-10},{50,-10},{
              50,-70},{-60,-70},{-60,40},{0,40}},
          fillColor={95,95,95},
          fillPattern=FillPattern.Solid,
          pattern=LinePattern.None,
          lineColor={0,0,0}),        Polygon(
          points={{-38,-44},{-34,-48},{42,28},{70,0},{70,60},{10,60},{38,32},{
              -38,-44}},
          fillColor={95,95,95},
          fillPattern=FillPattern.Solid,
          pattern=LinePattern.None,
          lineColor={0,0,0})}));
end ExternalLibraryObject;
