Converter
=========

Unit converter application

Converter was inspired by Josh Madison's Convert application (http://joshmadison.com/convert-for-windows/).  The major improvement is the ability to define custom categories and conversions.  All conversions are stored in an XML file, and only require that the user define one conversion per unit.  A graph structure is created and searched in order to map from one unit type to any other unit type in the same category, while requiring only a minimum number of conversion factors to be entered.

Currently, the expression parser is very weak and only works on simple conversion definitions (good enough for all conversions I've come across so far, including temperature conversion), but there is definitely some room for improvement there.

Feedback is welcome!  Please log bug and suggestions using GitHub's issue tracker.