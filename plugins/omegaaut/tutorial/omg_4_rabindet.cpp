/** @file pseudodet_example.cpp
 *
 * Example usage of pseudo-determinization algorithm for Rabin automata
 *
 * This example demonstrates how to use the PseudoDet function to convert
 * nondeterministic Rabin automata to deterministic ones.
 *
 * @ingroup Tutorials
 * @include pseudodet_example.cpp
 */

 #include "libfaudes.h"
 
 using namespace faudes;

 
 int main(void) {
     
     std::cout << "======== Pseudo-Determinization Example ========" << std::endl;

         // Read nondeterministic Rabin automaton from file
         RabinAutomaton NRA;
         NRA.Read("observed_belt.gen");
         
         std::cout << "\n=== Input Nondeterministic Rabin Automaton ===" << std::endl;
         NRA.DWrite();
         NRA.RabinGraphWrite("NRA.png");
         // Apply pseudo-determinization algorithm
         std::cout << "\n=== Applying Pseudo-Determinization Algorithm ===" << std::endl;

         RabinAutomaton DRA;
         PseudoDet(NRA, DRA);
         
         std::cout << "\n=== Output Deterministic Rabin Automaton ===" << std::endl;
         DRA.DWrite();
         DRA.RabinGraphWrite("DRA.png");
         return 0;
     }