#include "libfaudes.h"

using namespace faudes;

// Epsilon observation function: replace unobservable events with epsilon events
RabinAutomaton EpsObservation(const RabinAutomaton& rGen) {
    RabinAutomaton result = rGen;  // Copy original automaton
    
    // Get unobservable events
    EventSet unobservableEvents = result.UnobservableEvents();
    
    // If no unobservable events, return original automaton
    if(unobservableEvents.Size() == 0) {
        return result;
    }
    
    // Create or get epsilon event
    Idx epsEvent;
    if(result.ExistsEvent("eps")) {
        epsEvent = result.EventIndex("eps");
    } else {
        // Insert basic event first
        epsEvent = result.InsEvent("eps");
    }
    
    // Set epsilon event as neither controllable nor observable
    result.ClrControllable(epsEvent);
    result.ClrObservable(epsEvent);
    result.ClrForcible(epsEvent);
    
    // Replace all transitions using unobservable events
    TransSet originalTransitions = result.TransRel();  // Get copy of original transition relation
    
    TransSet::Iterator tit;
    for(tit = originalTransitions.Begin(); tit != originalTransitions.End(); ++tit) {
        if(unobservableEvents.Exists(tit->Ev)) {
            // This is a transition using an unobservable event
            
            // Remove original transition
            result.ClrTransition(tit->X1, tit->Ev, tit->X2);
            
            // Add new transition using epsilon
            result.SetTransition(tit->X1, epsEvent, tit->X2);
        }
    }
    
    // Remove unobservable events from alphabet
    EventSet::Iterator uit;
    for(uit = unobservableEvents.Begin(); uit != unobservableEvents.End(); ++uit) {
        result.DelEvent(*uit);
    }
    
    return result;
}

RabinAutomaton RabinProduct(const RabinAutomaton& rGen1, const RabinAutomaton& rGen2) {
    RabinAutomaton result;
    
    // 1. Construct alphabet intersection
    EventSet intersectAlphabet = rGen1.Alphabet() * rGen2.Alphabet();
    
    // 2. Create alphabet with attributes - inherit event controllability properties
    EventSet::Iterator evit;
    for(evit = intersectAlphabet.Begin(); evit != intersectAlphabet.End(); ++evit) {
        Idx event = *evit;
        
        // Directly inherit event attributes from plant (since event attributes are system-inherent)
        bool final_controllable = rGen1.Controllable(event);
        bool final_observable = rGen1.Observable(event);
        bool final_forcible = rGen1.Forcible(event);
        
        // Insert event with final attributes
        if(final_controllable) {
            result.InsControllableEvent(event);
        } else {
            result.InsUncontrollableEvent(event);
        }
        
        // Set observability
        if(final_observable) {
            result.SetObservable(event);
        } else {
            result.ClrObservable(event);
        }
        
        // Set forcibility
        if(final_forcible) {
            result.SetForcible(event);
        } else {
            result.ClrForcible(event);
        }
    }
    
    // 3-5. Construct states, transitions etc. (unchanged)
    std::map<std::pair<Idx, Idx>, Idx> stateMap;
    StateSet::Iterator sit1, sit2;
    for(sit1 = rGen1.StatesBegin(); sit1 != rGen1.StatesEnd(); ++sit1) {
        for(sit2 = rGen2.StatesBegin(); sit2 != rGen2.StatesEnd(); ++sit2) {
            std::string stateName = rGen1.StateName(*sit1) + "|" + rGen2.StateName(*sit2);
            Idx newState = result.InsState(stateName);
            stateMap[std::make_pair(*sit1, *sit2)] = newState;
        }
    }
    
    StateSet::Iterator init1, init2;
    for(init1 = rGen1.InitStatesBegin(); init1 != rGen1.InitStatesEnd(); ++init1) {
        for(init2 = rGen2.InitStatesBegin(); init2 != rGen2.InitStatesEnd(); ++init2) {
            result.SetInitState(stateMap[std::make_pair(*init1, *init2)]);
        }
    }
    
    StateSet::Iterator marked1, marked2;
    for(marked1 = rGen1.MarkedStatesBegin(); marked1 != rGen1.MarkedStatesEnd(); ++marked1) {
        for(marked2 = rGen2.MarkedStatesBegin(); marked2 != rGen2.MarkedStatesEnd(); ++marked2) {
            result.SetMarkedState(stateMap[std::make_pair(*marked1, *marked2)]);
        }
    }
    
    TransSet::Iterator tit1, tit2;
    for(tit1 = rGen1.TransRelBegin(); tit1 != rGen1.TransRelEnd(); ++tit1) {
        for(tit2 = rGen2.TransRelBegin(); tit2 != rGen2.TransRelEnd(); ++tit2) {
            if(tit1->Ev == tit2->Ev && intersectAlphabet.Exists(tit1->Ev)) {
                Idx srcState = stateMap[std::make_pair(tit1->X1, tit2->X1)];
                Idx dstState = stateMap[std::make_pair(tit1->X2, tit2->X2)];
                result.SetTransition(srcState, tit1->Ev, dstState);
            }
        }
    }
    
    // 6. Construct Rabin acceptance condition - ensure at least one empty pair to start the loop
    RabinAcceptance productAcc;
    RabinAcceptance acc1 = rGen1.RabinAcceptance();
    RabinAcceptance acc2 = rGen2.RabinAcceptance();
    
    // If any acceptance condition is empty, add an empty RabinPair
    if(acc1.Size() == 0) {
        RabinPair emptyPair;
        emptyPair.Name("empty1");
        acc1.Insert(emptyPair);
    }
    if(acc2.Size() == 0) {
        RabinPair emptyPair;
        emptyPair.Name("empty2");
        acc2.Insert(emptyPair);
    }
    
    // Now use unified logic to handle all cases
    RabinAcceptance::CIterator rit1, rit2;
    for(rit1 = acc1.Begin(); rit1 != acc1.End(); ++rit1) {
        for(rit2 = acc2.Begin(); rit2 != acc2.End(); ++rit2) {
            
            RabinPair newPair;
            
            // R1 × X2 (if R1 is empty, no states will be added here)
            StateSet::Iterator r1_it, x2_it;
            for(r1_it = rit1->RSet().Begin(); r1_it != rit1->RSet().End(); ++r1_it) {
                for(x2_it = rGen2.StatesBegin(); x2_it != rGen2.StatesEnd(); ++x2_it) {
                    Idx combinedState = stateMap[std::make_pair(*r1_it, *x2_it)];
                    newPair.RSet().Insert(combinedState);
                }
            }
            
            // X1 × R2 (if R2 is empty, no states will be added here)
            StateSet::Iterator x1_it, r2_it;
            for(x1_it = rGen1.StatesBegin(); x1_it != rGen1.StatesEnd(); ++x1_it) {
                for(r2_it = rit2->RSet().Begin(); r2_it != rit2->RSet().End(); ++r2_it) {
                    Idx combinedState = stateMap[std::make_pair(*x1_it, *r2_it)];
                    newPair.RSet().Insert(combinedState);
                }
            }
            
            // I1 × X2 (if I1 is empty, no states will be added here)
            StateSet::Iterator i1_it;
            for(i1_it = rit1->ISet().Begin(); i1_it != rit1->ISet().End(); ++i1_it) {
                for(x2_it = rGen2.StatesBegin(); x2_it != rGen2.StatesEnd(); ++x2_it) {
                    Idx combinedState = stateMap[std::make_pair(*i1_it, *x2_it)];
                    newPair.ISet().Insert(combinedState);
                }
            }
            
            // X1 × I2 (if I2 is empty, no states will be added here)
            StateSet::Iterator i2_it;
            for(x1_it = rGen1.StatesBegin(); x1_it != rGen1.StatesEnd(); ++x1_it) {
                for(i2_it = rit2->ISet().Begin(); i2_it != rit2->ISet().End(); ++i2_it) {
                    Idx combinedState = stateMap[std::make_pair(*x1_it, *i2_it)];
                    newPair.ISet().Insert(combinedState);
                }
            }
            
            newPair.Name(rit1->Name() + "_x_" + rit2->Name());
            productAcc.Insert(newPair);
        }
    }
    
    result.RabinAcceptance() = productAcc;
    
    return result;
}

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
    
    spec.Read("data/Rabinspec.gen");
    InvProject(spec,plant.Alphabet()); 
    spec.DWrite();
    
    // spec doesn't need event attribute settings, it only defines constraints
    
    // Compute product
    RabinAutomaton product = RabinProduct(plant, spec);
    product.DWrite();
    
    // Perform epsilon observation
    RabinAutomaton epsObserved = EpsObservation(product);
    epsObserved.Write();
    
    // Save results
    product.Write("data/product.gen");
    epsObserved.Write("data/eps_observed.gen");
    
    return 0;
}