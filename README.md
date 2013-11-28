Converter
=========

Converter is a customizable unit converter application.  It is licesned under the MIT license.  Please see the [wiki](https://github.com/KerryL/Converter/wiki/Home) for more detailed information.

Converter is built on wxWidgets.  It compiles and runs under both MSW and GTK.

Converter was inspired by Josh Madison's Convert application (http://joshmadison.com/convert-for-windows/).  The major improvement is the ability to define custom categories and conversions.  All conversions are stored in an XML file, and only require that the user define one conversion per unit.  A graph structure is created and searched in order to map from one unit type to any other unit type in the same category, while requiring only a minimum number of conversion factors to be entered.

Currently, the expression parser is very weak and only works on simple conversion definitions (good enough for all conversions I've come across so far, including temperature conversion), but there is definitely some room for improvement there.

Feedback is welcome!  Please log bugs and suggestions using GitHub's issue tracker.

[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/0fd4886340510dbe655700f724e6a41f "githalytics.com")](http://githalytics.com/KerryL/Converter)
