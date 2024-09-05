#include "tg4App.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
tg4App::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

tg4App::tg4App(InputParameters parameters) : MooseApp(parameters)
{
  tg4App::registerAll(_factory, _action_factory, _syntax);
}

tg4App::~tg4App() {}

void
tg4App::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAllObjects<tg4App>(f, af, s);
  Registry::registerObjectsTo(f, {"tg4App"});
  Registry::registerActionsTo(af, {"tg4App"});

  /* register custom execute flags, action syntax, etc. here */
}

void
tg4App::registerApps()
{
  registerApp(tg4App);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
tg4App__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  tg4App::registerAll(f, af, s);
}
extern "C" void
tg4App__registerApps()
{
  tg4App::registerApps();
}
