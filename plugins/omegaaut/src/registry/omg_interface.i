/** @file omg_interface.i @brief bindings for omegaaut plug-in */


/*
The omageaut plug-in does not provide bindings for data structures yet, so this
interface file is a dummy
*/

// Set SWIG module name
// Note: must match libFAUDES plugin name
%module omegaaut
#ifndef SwigModule
#define SwigModule "SwigOmegaAut"
#endif


// Load std faudes interface
%include "faudesmodule.i"

// Extra Lua functions: copy to faudes name space
#ifdef SWIGLUA
%luacode {
-- Copy synthesis to faudes name space
for k,v in pairs(omegaaut) do faudes[k]=v end
}
#endif				 




// extra c code for swig
%{
#include "omg_rabinaut.h"
#include "omg_rabinctrlpartialobs.h"
%}

/*
**************************************************
**************************************************
**************************************************

actual interface

**************************************************
**************************************************
**************************************************
*/

/* put interface of data structures here */

// RabinAutomaton class
%include "omg_rabinaut.h"

// SWIG specific director for RabinAutomaton (if needed for inheritance)
%feature("director") faudes::RabinAutomaton;


/*
**************************************************
**************************************************
**************************************************

Interface: auto generated

**************************************************
**************************************************
**************************************************
*/

// Add entry to mini help system: introduce new topic "priorities"
SwigHelpTopic("OmegaAut","Omega Automata PlugIn");

// Include rti generated function interface
#if SwigModule=="SwigOmegaAut"
%include "../../../include/rtiautoload.i"
#endif
 
