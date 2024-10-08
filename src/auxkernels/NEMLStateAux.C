/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifdef NEML_ENABLED

#include "NEMLStateAux.h"

registerMooseObject("tg4App", NEMLStateAux);

InputParameters
NEMLStateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<FileName>("database", "Path to NEML XML database.");
  params.addRequiredParam<std::string>("model", "Model name in NEML database.");
  params.addRequiredParam<std::string>("state_variable", "Name to store.");
  params.addParam<MaterialPropertyName>(
      "state_vector", "history", "Material property storing NEML state.");

  return params;
}

NEMLStateAux::NEMLStateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _fname(getParam<FileName>("database")),
    _mname(getParam<std::string>("model")),
    _neml_history(getMaterialProperty<std::vector<Real>>("state_vector")),
    _var_name(getParam<std::string>("state_variable"))
{
  // Check that the file is readable
  MooseUtils::checkFileReadable(_fname);

  // Will throw an exception if it doesn't succeed
  try
  {
    _model = neml::parse_xml_unique(_fname, _mname);
  }
  catch (const neml::NEMLError & e)
  {
    paramError("Unable to load NEML model " + _mname + " from file " + _fname);
  }

  // Get the list of names from neml
  auto names = _model->report_internal_variable_names();

  // Try to find the provided state_variable
  auto loc = std::find(names.begin(), names.end(), _var_name);

  // Check that it was in there
  if (loc == names.end())
    mooseError("The requested state variable was not an output of the "
               "provided NEML model");

  // Store the offset
  _offset = loc - names.begin();
}

Real
NEMLStateAux::computeValue()
{
  // Check that the vector we got has the right size for the model
  if (_model->nstore() != _neml_history[_qp].size())
    paramError("The size of the state_name vector provided to NEMLStateAux "
               "does not match the number of history variables requested "
               "by the NEML model itself.");

  // Trivial as we've done all the work in the constructor
  return _neml_history[_qp][_offset];
}

#endif // NEML_ENABLED
