//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML_ENABLED

#include "NEMLMaterialPropertyResetTest.h"

registerMooseObject("tg4App", NEMLMaterialPropertyResetTest);

InputParameters
NEMLMaterialPropertyResetTest::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  params.addCoupledVar("variable", "Coupled variable to trigger the reset");
  params.addRequiredParam<Real>("critical_value", "Value to trigger the reset at");

  params.addRequiredParam<std::vector<std::string>>("properties", "Properties to reset");
  params.addRequiredParam<MaterialName>("material", "The NEML material object to reset");

  return params;
}

NEMLMaterialPropertyResetTest::NEMLMaterialPropertyResetTest(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _variable(coupledValue("variable")),
    _critical_value(getParam<Real>("critical_value")),
    _props(getParam<std::vector<std::string>>("properties"))
{
}

void
NEMLMaterialPropertyResetTest::initialSetup()
{
  _neml_material = dynamic_cast<CauchyStressFromNEML *>(&getMaterial("material"));
  if (_neml_material == nullptr)
    mooseError("Unable to link NEMLMaterialPropertyResetTest object to the "
               "stress calculator");

  _indices = _neml_material->provide_indices(_props);
}

void
NEMLMaterialPropertyResetTest::initialize()
{
}

void
NEMLMaterialPropertyResetTest::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    resetQp();
}

void
NEMLMaterialPropertyResetTest::resetQp()
{
  if (_variable[_qp] >= _critical_value)
    _neml_material->reset_state(_indices, _qp);
}

void
NEMLMaterialPropertyResetTest::finalize()
{
}

void
NEMLMaterialPropertyResetTest::threadJoin(const UserObject & /*y*/)
{
}

#endif // NEML_ENABLED
