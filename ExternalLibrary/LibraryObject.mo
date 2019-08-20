within ExternalLibrary;
model LibraryObject

  parameter String filename=Modelica.Utilities.Files.loadResource("modelica://ExternalLibrary/Resources/Data/data.txt");
  parameter String pythonHome=Modelica.Utilities.System.getEnvironmentVariable("PYTHONHOME");

  Modelica.Blocks.Interfaces.RealInput u[nin] "Connector of Real input signal"
    annotation (Placement(transformation(extent={{-140,-20},{-100,20}})));

  Modelica.Blocks.Interfaces.RealOutput y[nout]
    "Connector of Real output signal"
    annotation (Placement(transformation(extent={{100,-10},{120,10}})));

  ExternalLibrary.Internal.ExternalLibraryObject externalObject=
      ExternalLibrary.Internal.ExternalLibraryObject(filename, pythonHome);

protected
  constant Integer nin=2;
  constant Integer nout=2;

  function evaluate
    input ExternalLibrary.Internal.ExternalLibraryObject externalObject;
    input Real[nin] u_;
    output Real[nout] y_;
  external"C" evaluateExternalObject(
        externalObject,
        size(u_, 1),
        u_,
        size(y_, 1),
        y_) annotation (Library="ExternalLibrary");
  end evaluate;

equation
  y = evaluate(externalObject, u);

  annotation (Icon(graphics={Rectangle(
          extent={{-100,-100},{100,100}},
          lineColor={0,0,127},
          fillColor={255,255,255},
          fillPattern=FillPattern.Solid)}), experiment(
      StopTime=10,
      __Dymola_fixedstepsize=0.05,
      __Dymola_Algorithm="Euler"));
end LibraryObject;
