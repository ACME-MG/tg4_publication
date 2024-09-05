//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "tg4TestApp.h"
#include "tg4App.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
tg4TestApp::validParams()
{
  InputParameters params = tg4App::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

tg4TestApp::tg4TestApp(InputParameters parameters) : MooseApp(parameters)
{
  tg4TestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

tg4TestApp::~tg4TestApp() {}

void
tg4TestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  tg4App::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"tg4TestApp"});
    Registry::registerActionsTo(af, {"tg4TestApp"});
  }
}

void
tg4TestApp::registerApps()
{
  registerApp(tg4App);
  registerApp(tg4TestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
tg4TestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  tg4TestApp::registerAll(f, af, s);
}
extern "C" void
tg4TestApp__registerApps()
{
  tg4TestApp::registerApps();
}
