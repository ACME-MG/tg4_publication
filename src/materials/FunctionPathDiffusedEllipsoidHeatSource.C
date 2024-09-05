//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionPathDiffusedEllipsoidHeatSource.h"

#include "Function.h"

registerMooseObject("tg4App", FunctionPathDiffusedEllipsoidHeatSource);

InputParameters
FunctionPathDiffusedEllipsoidHeatSource::validParams()
{
  InputParameters params = Material::validParams();

  /// heat source path
  params.addParam<Real>("path_x", 0, "The x component of the heat source centre");
  params.addParam<Real>("path_y", 0, "The y component of the heat source centre");
  params.addParam<Real>("path_z", 0, "The z component of the heat source centre");
  params.addParam<FunctionName>(
      "function_path_x", "0", "The x component of the heat source centre as a function of time");
  params.addParam<FunctionName>(
      "function_path_y", "0", "The y component of the heat source centre as a function of time");
  params.addParam<FunctionName>(
      "function_path_z", "0", "The z component of the heat source centre as a function of time");

  /// heat source dimensions
  params.addParam<Real>("rx", 0, "Ellipsoid radius in x direction");
  params.addParam<Real>("ry", 0, "Ellipsoid radius in y direction");
  params.addParam<Real>("rz", 0, "Ellipsoid radius in z direction");
  params.addParam<FunctionName>(
      "function_rx", "0", "Ellipsoid radius in x direction as a function of time");
  params.addParam<FunctionName>(
      "function_ry", "0", "Ellipsoid radius in y direction as a function of time");
  params.addParam<FunctionName>(
      "function_rz", "0", "Ellipsoid radius in z direction as a function of time");

  /// heat source power
  params.addParam<Real>("power", 0, "Heat source power");
  params.addParam<Real>("efficiency", 1, "Heat source efficiency");
  params.addParam<FunctionName>("function_power", "0", "Heat source power as a function of time");
  params.addParam<FunctionName>(
      "function_efficiency", "1", "Heat source efficiency as a function of time");

  /// heat source tilt
  params.addParam<Real>("tilt", 0, "The clockwise tilt of the heat source around the z axis");
  params.addParam<FunctionName>(
      "function_tilt",
      "0",
      "The clockwise tilt of the heat source around the z axis as a function of time");

  /// heat source weave
  params.addParam<Real>("weave_amp_x", 0, "The amplitude of the weave in the x direction");
  params.addParam<FunctionName>(
      "function_weave_amp_x",
      "0",
      "The amplitude of the weave in the x direction as a function of time");
  params.addParam<Real>("weave_amp_y", 0, "The amplitude of the weave in the y direction");
  params.addParam<FunctionName>(
      "function_weave_amp_y",
      "0",
      "The amplitude of the weave in the y direction as a function of time");
  params.addParam<Real>("weave_amp_z", 0, "The amplitude of the weave in the z direction");
  params.addParam<FunctionName>(
      "function_weave_amp_z",
      "0",
      "The amplitude of the weave in the z direction as a function of time");

  /// half model
  params.addParam<bool>("half_model", false, "The simulation uses a half model");

  /// volumetric heat source calculation
  params.addParam<Real>("va", 0, "va value to determine volumetric heat.");
  params.addParam<FunctionName>(
      "function_va", "0", "va value to determine volumetric heat as function of time");
  params.addParam<PostprocessorName>(
      "pp_va", "1", "Postprocessor with va value to determine volumetric heat.");

  params.addClassDescription("Diffused oldak ellipsoid volumetric heat source with varying parameters and function path.");
  return params;
}

FunctionPathDiffusedEllipsoidHeatSource::FunctionPathDiffusedEllipsoidHeatSource(
    const InputParameters & parameters)
  : Material(parameters),

    _path_x(getParam<Real>("path_x")),
    _path_y(getParam<Real>("path_y")),
    _path_z(getParam<Real>("path_z")),
    _function_path_x(getFunction("function_path_x")),
    _function_path_y(getFunction("function_path_y")),
    _function_path_z(getFunction("function_path_z")),

    _rx(getParam<Real>("rx")),
    _ry(getParam<Real>("ry")),
    _rz(getParam<Real>("rz")),
    _function_rx(getFunction("function_rx")),
    _function_ry(getFunction("function_ry")),
    _function_rz(getFunction("function_rz")),

    _power(getParam<Real>("power")),
    _efficiency(getParam<Real>("efficiency")),
    _function_power(getFunction("function_power")),
    _function_efficiency(getFunction("function_efficiency")),

    _tilt(getParam<Real>("tilt")),
    _function_tilt(getFunction("function_tilt")),

    _weave_amp_x(getParam<Real>("weave_amp_x")),
    _function_weave_amp_x(getFunction("function_weave_amp_x")),
    _weave_amp_y(getParam<Real>("weave_amp_y")),
    _function_weave_amp_y(getFunction("function_weave_amp_y")),
    _weave_amp_z(getParam<Real>("weave_amp_z")),
    _function_weave_amp_z(getFunction("function_weave_amp_z")),

    _half_model(getParam<bool>("half_model")),

    _calc_va(declareADProperty<Real>("calc_va")),
    _va(getParam<Real>("va")),
    _function_va(getFunction("function_va")),
    _pp_va(getPostprocessorValue("pp_va")),
    _volumetric_heat(declareADProperty<Real>("volumetric_heat"))
{
  // Parameters cannot take both a value and a function, and parameters are required
  if (isParamSetByUser("path_x") && isParamSetByUser("function_path_x"))
  {
    mooseError("Cannot set both path_x and function_path_x");
  }
  else if (!isParamSetByUser("path_x") && !isParamSetByUser("function_path_x"))
  {
    mooseError("No path_x or function_path_x defined");
  }

  if (isParamSetByUser("path_y") && isParamSetByUser("function_path_y"))
  {
    mooseError("Cannot set both path_y and function_path_y");
  }
  else if (!isParamSetByUser("path_y") && !isParamSetByUser("function_path_y"))
  {
    mooseError("No path_y or function_path_y defined");
  }

  if (isParamSetByUser("path_z") && isParamSetByUser("function_path_z"))
  {
    mooseError("Cannot set both path_z and function_path_z");
  }
  else if (!isParamSetByUser("path_z") && !isParamSetByUser("function_path_z"))
  {
    mooseError("No path_z or function_path_z defined");
  }

  if (isParamSetByUser("rx") && isParamSetByUser("function_rx"))
  {
    mooseError("Cannot set both rx and function_rx");
  }
  else if (!isParamSetByUser("rx") && !isParamSetByUser("function_rx"))
  {
    mooseError("No rx or function_rx defined");
  }

  if (isParamSetByUser("ry") && isParamSetByUser("function_ry"))
  {
    mooseError("Cannot set both ry and function_ry");
  }
  else if (!isParamSetByUser("ry") && !isParamSetByUser("function_ry"))
  {
    mooseError("No ry or function_ry defined");
  }

  if (isParamSetByUser("rz") && isParamSetByUser("function_rz"))
  {
    mooseError("Cannot set both rz and function_rz");
  }
  else if (!isParamSetByUser("rz") && !isParamSetByUser("function_rz"))
  {
    mooseError("No rz or function_rz defined");
  }

  if (isParamSetByUser("power") && isParamSetByUser("function_power"))
  {
    mooseError("Cannot set both power and function_power");
  }
  else if (!isParamSetByUser("power") && !isParamSetByUser("function_power"))
  {
    mooseError("No power or function_power defined");
  }

  // Parameters cannot take both a value and a function, and parameters are not required
  if (isParamSetByUser("efficiency") && isParamSetByUser("function_efficiency"))
  {
    mooseError("Cannot set both efficiency and function_efficiency");
  }

  if (isParamSetByUser("tilt") && isParamSetByUser("function_tilt"))
  {
    mooseError("Cannot set both tilt and function_tilt");
  }

  if (isParamSetByUser("weave_amp_x") && isParamSetByUser("function_weave_amp_x"))
  {
    mooseError("Cannot set both weave_amp_x and function_weave_amp_x");
  }
  if (isParamSetByUser("weave_amp_y") && isParamSetByUser("function_weave_amp_y"))
  {
    mooseError("Cannot set both weave_amp_x and function_weave_amp_x");
  }
  if (isParamSetByUser("weave_amp_z") && isParamSetByUser("function_weave_amp_z"))
  {
    mooseError("Cannot set both weave_amp_x and function_weave_amp_x");
  }

  if (isParamSetByUser("va") && isParamSetByUser("function_va"))
  {
    mooseError("Cannot set both va and function_va");
  }
  else if (isParamSetByUser("va") && isParamSetByUser("pp_va"))
  {
    mooseError("Cannot set both va and pp_va");
  }
  else if (isParamSetByUser("function_va") && isParamSetByUser("pp_va"))
  {
    mooseError("Cannot set both function_va and pp_va");
  }
  else if (isParamSetByUser("va") && isParamSetByUser("function_va") && isParamSetByUser("pp_va"))
  {
    mooseError("Cannot set va, function_va and pp_va");
  }
  else if (!isParamSetByUser("va") && !isParamSetByUser("function_va") &&
           !isParamSetByUser("pp_va"))
  {
    mooseError("No va, function_va or pp_va defined");
  }
}

void
FunctionPathDiffusedEllipsoidHeatSource::computeQpProperties()
{
  // Set variables for parameter values
  Real path_x_t;
  Real path_y_t;
  Real path_z_t;
  Real rx_t;
  Real ry_t;
  Real rz_t;
  Real p_t;
  Real eta_t;
  Real tilt_t;
  Real weave_x_t;
  Real weave_y_t;
  Real weave_z_t;

  // Coordinates of the quadrature point
  const Real & x = _q_point[_qp](0);
  const Real & y = _q_point[_qp](1);
  const Real & z = _q_point[_qp](2);

  // Set parameter value from user value or function
  if (isParamSetByUser("path_x"))
  {
    path_x_t = _path_x;
  }
  else
  {
    path_x_t = _function_path_x.value(_t);
  }

  if (isParamSetByUser("path_y"))
  {
    path_y_t = _path_y;
  }
  else
  {
    path_y_t = _function_path_y.value(_t);
  }

  if (isParamSetByUser("path_z"))
  {
    path_z_t = _path_z;
  }
  else
  {
    path_z_t = _function_path_z.value(_t);
  }

  if (isParamSetByUser("rx"))
  {
    rx_t = _rx;
  }
  else
  {
    rx_t = _function_rx.value(_t);
  }

  if (isParamSetByUser("ry"))
  {
    ry_t = _ry;
  }
  else
  {
    ry_t = _function_ry.value(_t);
  }

  if (isParamSetByUser("rz"))
  {
    rz_t = _rz;
  }
  else
  {
    rz_t = _function_rz.value(_t);
  }

  if (isParamSetByUser("power"))
  {
    p_t = _power;
  }
  else
  {
    p_t = _function_power.value(_t);
  }

  // Set parameter value from user value or function, or default if neither given
  if (isParamSetByUser("efficiency"))
  {
    eta_t = _efficiency;
  }
  else if (isParamSetByUser("function_efficiency"))
  {
    eta_t = _function_efficiency.value(_t);
  }
  else
  {
    eta_t = 1.0;
  }

  if (isParamSetByUser("tilt"))
  {
    tilt_t = _tilt;
  }
  else if (isParamSetByUser("function_tilt"))
  {
    tilt_t = _function_tilt.value(_t);
  }
  else
  {
    tilt_t = 0.0;
  }

  if (isParamSetByUser("weave_amp_x"))
  {
    weave_x_t = _weave_amp_x;
  }
  else if (isParamSetByUser("function_weave_amp_x"))
  {
    weave_x_t = _function_weave_amp_x.value(_t);
  }
  else
  {
    weave_x_t = 0.0;
  }

  if (isParamSetByUser("weave_amp_y"))
  {
    weave_y_t = _weave_amp_y;
  }
  else if (isParamSetByUser("function_weave_amp_y"))
  {
    weave_y_t = _function_weave_amp_y.value(_t);
  }
  else
  {
    weave_y_t = 0.0;
  }

  if (isParamSetByUser("weave_amp_z"))
  {
    weave_z_t = _weave_amp_z;
  }
  else if (isParamSetByUser("function_weave_amp_z"))
  {
    weave_z_t = _function_weave_amp_z.value(_t);
  }
  else
  {
    weave_z_t = 0.0;
  }

  // rotate the coordinate system anticlockwise around the z axis
  // Real x_rot = (x - path_x_t) * std::cos(tilt_t) + (y - path_y_t) * std::sin(tilt_t);
  // Real y_rot = -(x - path_x_t) * std::sin(tilt_t) + (y - path_y_t) * std::cos(tilt_t);

  // rotate the coordinate system clockwise around the z axis
  Real x_rot = (x - path_x_t) * std::cos(tilt_t) - (y - path_y_t) * std::sin(tilt_t);
  Real y_rot = (x - path_x_t) * std::sin(tilt_t) + (y - path_y_t) * std::cos(tilt_t);

  Real calc_va_temp = 0.0;

  if (weave_x_t > 1e-6 || weave_y_t > 1e-6 || weave_z_t > 1e-6)
  {
    for (int i = 0; i <= 12; ++i)
    {
      if (weave_x_t > 1e-6)
      {
        calc_va_temp += std::exp(-(
            std::pow(x_rot + weave_x_t * std::sin(libMesh::pi * i / 6), 2.0) / std::pow(rx_t, 2.0) +
            std::pow(y_rot, 2.0) / std::pow(ry_t, 2.0) +
            std::pow(z - path_z_t, 2.0) / std::pow(rz_t, 2.0)));
      }
      if (weave_y_t > 1e-6)
      {
        calc_va_temp += std::exp(-(
            std::pow(x_rot, 2.0) / std::pow(rx_t, 2.0) +
            std::pow(y_rot + weave_y_t * std::sin(libMesh::pi * i / 6), 2.0) / std::pow(ry_t, 2.0) +
            std::pow(z - path_z_t, 2.0) / std::pow(rz_t, 2.0)));
      }
      if (weave_z_t > 1e-6)
      {
        calc_va_temp +=
            std::exp(-(std::pow(x_rot, 2.0) / std::pow(rx_t, 2.0) +
                       std::pow(y_rot, 2.0) / std::pow(ry_t, 2.0) +
                       std::pow(z - path_z_t + weave_z_t * std::sin(libMesh::pi * i / 6), 2.0) /
                           std::pow(rz_t, 2.0)));
      }
    }
  }
  else
  {
    calc_va_temp += std::exp(-(std::pow(x_rot, 2.0) / std::pow(rx_t, 2.0) +
                               std::pow(y_rot, 2.0) / std::pow(ry_t, 2.0) +
                               std::pow(z - path_z_t, 2.0) / std::pow(rz_t, 2.0)));
  }

  _calc_va[_qp] = calc_va_temp;

  if (isParamSetByUser("va"))
  {
    if (_half_model)
    {
      _volumetric_heat[_qp] = 0.5 * p_t * eta_t * _calc_va[_qp] / _va;
    }
    else
    {
      _volumetric_heat[_qp] = p_t * eta_t * _calc_va[_qp] / _va;
    }
  }
  else if (isParamSetByUser("function_va"))
  {
    if (_half_model)
    {
      _volumetric_heat[_qp] = 0.5 * p_t * eta_t * _calc_va[_qp] / _function_va.value(_t);
    }
    else
    {
      _volumetric_heat[_qp] = p_t * eta_t * _calc_va[_qp] / _function_va.value(_t);
    }
  }
  else
  {
    if (_half_model)
    {
      _volumetric_heat[_qp] = 0.5 * p_t * eta_t * _calc_va[_qp] / _pp_va;
    }
    else
    {
      _volumetric_heat[_qp] = p_t * eta_t * _calc_va[_qp] / _pp_va;
    }
  }
}
