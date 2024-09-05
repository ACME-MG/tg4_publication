//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class Function;

/**
 * Goldak ellipsoid heat source distribution.
 */
class FunctionPathEllipsoidHeatSourceRamp : public Material
{
public:
  static InputParameters validParams();

  FunctionPathEllipsoidHeatSourceRamp(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// heat source path
  const Real _path_x;
  const Real _path_y;
  const Real _path_z;

  const Function & _function_path_x;
  const Function & _function_path_y;
  const Function & _function_path_z;

  /// heat source dimensions
  const Real _rx;
  const Real _ry;
  const Real _rz;

  const Function & _function_rx;
  const Function & _function_ry;
  const Function & _function_rz;

  /// heat source power
  const Real _power;
  const Real _efficiency;

  const Function & _function_power;
  const Function & _function_efficiency;

  /// heat source tilt
  const Real _tilt;
  const Function & _function_tilt;

  /// heat source weaves
  const Real _weave_amp_x;
  const Function & _function_weave_amp_x;
  const Real _weave_amp_y;
  const Function & _function_weave_amp_y;
  const Real _weave_amp_z;
  const Function & _function_weave_amp_z;

  /// half model
  const bool _half_model;

  /// volumetric heat source calculation
  ADMaterialProperty<Real> & _calc_va;
  const Real _va;
  const Function & _function_va;
  const PostprocessorValue & _pp_va;
  ADMaterialProperty<Real> & _volumetric_heat;

};
