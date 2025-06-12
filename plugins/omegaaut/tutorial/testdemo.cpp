#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;



int main() {


  // Compose plant dynamics from two very simple machines 
  Generator tempgen, machinea, machineb;
  System cplant; 
 
  tempgen.Read("data/cbplant.gen");
  tempgen.Version("1",machinea);
  tempgen.Version("2",machineb);
  Parallel(machinea,machineb,cplant);

  // Declare controllable events
  EventSet contevents;
  contevents.Insert("a_1");
  cplant.SetControllable(contevents);
  
  EventSet allEvents = cplant.Alphabet();
  cplant.ClrObservable(allEvents);

  EventSet obsevents;
  obsevents.Insert("a_1");
  obsevents.Insert("b_2");
  cplant.SetObservable(obsevents);
  // Write to file
  cplant.Write("tmp_doublebelts.gen");

  // Report to console
  cplant.Name("two belts");
  cplant.DWrite();

  // Read spec 
  RabinAutomaton spec;
  spec.Read("data/cbspec.gen");
  spec.Name("spec belt");
  spec.Write("spec_belt.gen");

  InvProject(spec,cplant.Alphabet()); 
  spec.DWrite();

  RabinAutomaton product = RabinProduct(cplant, spec);
    product.DWrite();
    
    RabinAutomaton expandedplant = ControlPatternGenerator::ExpandToControlPatterns(
        product, contevents);
    
    std::cout << "\nExpanded Buffer Automaton:" << std::endl;
    expandedplant.DWrite();
    
    RabinAutomaton epsObserved = EpsObservation(expandedplant);
    epsObserved.DWrite();
    epsObserved.Write("observed_belt.gen");
    return 0;
}