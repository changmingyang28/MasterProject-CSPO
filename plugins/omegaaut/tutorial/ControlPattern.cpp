#include "libfaudes.h"

using namespace faudes;

int main(void){
    RabinAutomaton plant, spec;
    plant.Read("data/Rabinplant.gen");
    
    // Set controllable events
    EventSet contevents;
    contevents.Insert("alpha_1");
    contevents.Insert("alpha_2");
    plant.SetControllable(contevents);
    
    // Set observable events (only alpha_1 and alpha_2 are observable)
    // First set all events as unobservable
    EventSet allEvents = plant.Alphabet();
    plant.ClrObservable(allEvents);
    
    // Then set only alpha_1 and alpha_2 as observable
    EventSet obsevents;
    obsevents.Insert("alpha_1");
    obsevents.Insert("alpha_2");
    plant.SetObservable(obsevents);

    plant.DWrite();
    plant.ObservableEvents();
    spec.Read("data/Rabinspec.gen");
    InvProject(spec,plant.Alphabet()); 
    spec.DWrite();
    
    // spec doesn't need event attribute settings, it only defines constraints
    
    // Compute product
    RabinAutomaton product = RabinProduct(plant, spec);
    product.DWrite();
    
    RabinAutomaton expandedplant = ControlPatternGenerator::ExpandToControlPatterns(
        product, contevents);
    
    std::cout << "\nExpanded Buffer Automaton:" << std::endl;
    expandedplant.DWrite();
    
    RabinAutomaton epsObserved = EpsObservation(expandedplant);
    epsObserved.DWrite();
    
    return 0;
}
