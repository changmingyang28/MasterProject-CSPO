#include "libfaudes.h"

using namespace faudes;

int main() {
    // Create the same plant as in omg_6_rabinctrl
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

    // Debug: print extracted events
    std::cout << "Manual check:" << std::endl;
    std::cout << "Alphabet: " << cplant.Alphabet().ToString() << std::endl;
    std::cout << "Controllable events: " << cplant.ControllableEvents().ToString() << std::endl;
    std::cout << "Observable events: " << cplant.ObservableEvents().ToString() << std::endl;

    // Check individual events
    EventSet::Iterator it = cplant.Alphabet().Begin();
    for(; it != cplant.Alphabet().End(); ++it) {
        std::cout << "Event " << cplant.EventName(*it) << " [" << *it << "]: ";
        if(cplant.Controllable(*it)) std::cout << " controllable";
        if(cplant.Observable(*it)) std::cout << " observable";
        std::cout << std::endl;
    }

    return 0;
}