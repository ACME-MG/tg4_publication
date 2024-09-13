//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML_ENABLED

#include "NEMLMaterialPropertyReset.h"

registerMooseObject("tg4App", NEMLMaterialPropertyReset);

InputParameters
NEMLMaterialPropertyReset::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  params.addCoupledVar("variable", "Coupled variable to trigger the reset");
  params.addParam<bool>("two_stage", false, "Two-stage reset sets a constant state at a lower value and resets at an upper value");
  params.addParam<Real>("critical_value", 0, "Value to trigger reset at");
  params.addParam<Real>("lower_value", 0, "Lower value to trigger constant state at for two-stage reset");
  params.addParam<Real>("upper_value", 0, "Upper value to trigger reset at for two-stage reset");

  params.addRequiredParam<std::vector<std::string>>("properties", "Properties to reset");
  params.addRequiredParam<MaterialName>("material", "The NEML material object to reset");

  return params;
}

NEMLMaterialPropertyReset::NEMLMaterialPropertyReset(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _variable(coupledValue("variable")),
    _two_stage(getParam<bool>("two_stage")),
    _critical_value(getParam<Real>("critical_value")),
    _lower_value(getParam<Real>("lower_value")),
    _upper_value(getParam<Real>("upper_value")),
    _props(getParam<std::vector<std::string>>("properties"))
{
}

void
NEMLMaterialPropertyReset::initialSetup()
{
  _neml_material = dynamic_cast<CauchyStressFromNEML *>(&getMaterial("material"));
  if (_neml_material == nullptr)
    mooseError("Unable to link NEMLMaterialPropertyReset object to the "
               "stress calculator");

  _indices = _neml_material->provide_indices(_props);
  
  /// Errors for single-stage reset
  if (!_two_stage)
  {
    if (isParamSetByUser("lower_value") || isParamSetByUser("upper_value"))
    {
      mooseError("Lower and upper values should not be set for single-stage reset. Set critical value instead.");
    }
    else if (!isParamSetByUser("critical_value"))
    {
      mooseError("Critical value must be set for single-stage reset");
    }
  }

  /// Errors for two-stage reset
  if (_two_stage)
  {
    if (_lower_value >= _upper_value)
    {
      mooseError("Lower value must be less than upper value for two-stage reset");
    }
    else if (isParamSetByUser("lower_value") && !isParamSetByUser("upper_value"))
    {
      mooseError("Upper value must be set for two-stage reset");
    }
    else if (!isParamSetByUser("lower_value") && isParamSetByUser("upper_value"))
    {
      mooseError("Lower value must be set for two-stage reset");
    }
    else if (!isParamSetByUser("lower_value") && !isParamSetByUser("upper_value"))
    {
      mooseError("Lower and upper values must be set for two-stage reset");
    }
    else if (isParamSetByUser("critical_value"))
    {
      mooseError("Critical value should not be set for two-stage reset. Set lower and upper values instead.");
    }
  }
}

void
NEMLMaterialPropertyReset::initialize()
{
}

void
NEMLMaterialPropertyReset::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    resetQp();
}

void
NEMLMaterialPropertyReset::resetQp()
{
  if (!_two_stage)
  {
    if (_variable[_qp] >= _critical_value)
    {
      _neml_material->reset_state(_indices, _qp);
    }
  }
  else if (_two_stage)
  {
    if (_variable[_qp] >= _upper_value)
    {
      _neml_material->reset_state(_indices, _qp);
    }
    else if (_variable[_qp] >= _lower_value && _variable[_qp] < _upper_value)
    {
      _neml_material->const_state(_indices, _qp);
    }
  }
}

void
NEMLMaterialPropertyReset::finalize()
{
}

void
NEMLMaterialPropertyReset::threadJoin(const UserObject & /*y*/)
{
}

#endif // NEML_ENABLED
