#ifndef CONTROL_PATTERN_H
#define CONTROL_PATTERN_H

#include "libfaudes.h"
#include "omg_rabinaut.h"  // For RabinAutomaton typedef

namespace faudes {

/**
 * @brief Augmented event that includes both the event and its control pattern
 * 
 * This class extends AttributeVoid to represent an event paired with 
 * the control pattern under which it can occur.
 */
class FAUDES_API AugmentedEvent : public AttributeVoid {

FAUDES_TYPE_DECLARATION(Void, AugmentedEvent, AttributeVoid)

public:
    /**
     * @brief Default constructor
     */
    AugmentedEvent(void);
    
    /**
     * @brief Constructor with event and control pattern
     * @param event The event index
     * @param pattern The control pattern (set of allowed events)
     */
    AugmentedEvent(Idx event, const EventSet& pattern);
    
    /**
     * @brief Copy constructor
     */
    AugmentedEvent(const AugmentedEvent& other);
    
    /**
     * @brief Destructor
     */
    virtual ~AugmentedEvent(void) {};
    
    /**
     * @brief Get the event index
     * @return Event index
     */
    Idx Event(void) const { return mEvent; }
    
    /**
     * @brief Set the event index
     * @param event Event index to set
     */
    void Event(Idx event) { mEvent = event; }
    
    /**
     * @brief Get the control pattern
     * @return Reference to the control pattern
     */
    const EventSet& ControlPattern(void) const { return mControlPattern; }
    
    /**
     * @brief Set the control pattern
     * @param pattern Control pattern to set
     */
    void ControlPattern(const EventSet& pattern) { mControlPattern = pattern; }
    
    /**
     * @brief Comparison operator for ordering
     * @param other Other AugmentedEvent to compare with
     * @return true if this < other
     */
    bool operator<(const AugmentedEvent& other) const;

protected:
    /**
     * @brief Assignment method
     * @param rSrc Source to assign from
     */
    void DoAssign(const AugmentedEvent& rSrc);
    
    /**
     * @brief Test equality
     * @param rOther Other AugmentedEvent to compare with
     * @return true if equal
     */
    bool DoEqual(const AugmentedEvent& rOther) const;
    
    /**
     * @brief Write to TokenWriter
     * @param rTw TokenWriter to write to
     * @param rLabel Section to write
     * @param pContext Context information
     */
    virtual void DoWrite(TokenWriter& rTw, const std::string& rLabel="", const Type* pContext=nullptr) const;
    
    /**
     * @brief Read from TokenReader
     * @param rTr TokenReader to read from
     * @param rLabel Section to read
     * @param pContext Context information
     */
    virtual void DoRead(TokenReader& rTr, const std::string& rLabel="", const Type* pContext=nullptr);

private:
    /// The event index
    Idx mEvent;
    
    /// The control pattern (set of events that can be enabled together)
    EventSet mControlPattern;
};

/**
 * @brief Alphabet of augmented events
 * 
 * This class manages a collection of AugmentedEvent objects,
 * representing all possible event-control pattern combinations.
 */
class FAUDES_API AugmentedAlphabet : public TBaseSet<AugmentedEvent> {

FAUDES_TYPE_DECLARATION(Void, AugmentedAlphabet, TBaseSet<AugmentedEvent>)

public:
    /**
     * @brief Default constructor
     */
    AugmentedAlphabet(void);
    
    /**
     * @brief Copy constructor
     */
    AugmentedAlphabet(const AugmentedAlphabet& other);
    
    /**
     * @brief Destructor
     */
    virtual ~AugmentedAlphabet(void) {};
    
    /**
     * @brief Insert an augmented event (wrapper for convenience)
     * @param event Event index
     * @param pattern Control pattern
     * @return true if inserted successfully
     */
    bool InsertAugmentedEvent(Idx event, const EventSet& pattern);
    
    /**
     * @brief Check if a specific event-pattern combination exists
     * @param event Event index
     * @param pattern Control pattern
     * @return true if exists
     */
    bool ExistsAugmentedEvent(Idx event, const EventSet& pattern) const;
    
    /**
     * @brief Find all augmented events with a specific control pattern
     * @param pattern Control pattern
     * @return Set of augmented events with the specified pattern
     */
    AugmentedAlphabet FindByPattern(const EventSet& pattern) const;
    
    /**
     * @brief Get all unique events in this alphabet
     * @return EventSet containing all unique events
     */
    EventSet Events(void) const;
    
    /**
     * @brief Get all unique control patterns in this alphabet
     * @return Vector of unique control patterns
     */
    std::vector<EventSet> ControlPatterns(void) const;

protected:
    /**
     * @brief Assignment method
     * @param rSrc Source to assign from
     */
    void DoAssign(const AugmentedAlphabet& rSrc);
};

/**
 * @brief Control Pattern Generator
 * 
 * Utility class to generate all valid control patterns for a given alphabet
 * based on controllability information.
 */
class FAUDES_API ControlPatternGenerator {

public:
    /**
     * @brief Generate all valid control patterns
     * @param alphabet Full event alphabet
     * @param controllableEvents Set of controllable events
     * @return Vector of all valid control patterns
     */
    static std::vector<EventSet> GenerateControlPatterns(
        const EventSet& alphabet, 
        const EventSet& controllableEvents
    );
    
    /**
     * @brief Generate augmented alphabet from original alphabet
     * @param alphabet Original event alphabet
     * @param controllableEvents Set of controllable events
     * @return AugmentedAlphabet containing all event-pattern combinations
     */
    static AugmentedAlphabet GenerateAugmentedAlphabet(
        const EventSet& alphabet,
        const EventSet& controllableEvents
    );
    
    /**
     * @brief Expand Rabin automaton alphabet to control patterns
     * @param rGen Original Rabin automaton
     * @param controllableEvents Set of controllable events in the automaton
     * @return New Rabin automaton with expanded alphabet and transitions
     */
    static RabinAutomaton ExpandToControlPatterns(
        const RabinAutomaton& rGen,
        const EventSet& controllableEvents
    );

private:
    /**
     * @brief Generate all subsets of controllable events plus uncontrollable events
     * @param controllableEvents Set of controllable events
     * @param uncontrollableEvents Set of uncontrollable events
     * @param result Vector to store results
     * @param current Current subset being built
     * @param index Current index in controllable events
     */
    static void GenerateSubsets(
        const std::vector<Idx>& controllableEvents,
        const EventSet& uncontrollableEvents,
        std::vector<EventSet>& result,
        EventSet& current,
        size_t index
    );
};

// ============================================================================
// Global Functions for Rabin Automata Operations
// ============================================================================

/**
 * @brief Epsilon observation for Rabin automata
 * 
 * This function performs epsilon observation on a Rabin automaton by replacing
 * all unobservable events with corresponding epsilon events. Each control pattern
 * gets its own epsilon event to maintain the control pattern structure.
 * 
 * @param rGen Input Rabin automaton
 * @return New Rabin automaton with epsilon events replacing unobservable events
 */
FAUDES_API RabinAutomaton EpsObservation(const RabinAutomaton& rGen);

/**
 * @brief Compute the synchronous product of two Rabin automata
 * 
 * This function computes the synchronous product (parallel composition) of two
 * Rabin automata. The result is a new Rabin automaton that represents the
 * concurrent behavior of both input automata.
 * 
 * @param rGen1 First Rabin automaton
 * @param rGen2 Second Rabin automaton  
 * @return New Rabin automaton representing the synchronous product
 */
FAUDES_API RabinAutomaton RabinProduct(const RabinAutomaton& rGen1, const RabinAutomaton& rGen2);

// ============================================================================
// Inline Implementations
// ============================================================================

inline bool AugmentedEvent::operator<(const AugmentedEvent& other) const {
    if (mEvent != other.mEvent) {
        return mEvent < other.mEvent;
    }
    return mControlPattern < other.mControlPattern;
}

} // namespace faudes

#endif // CONTROL_PATTERN_H