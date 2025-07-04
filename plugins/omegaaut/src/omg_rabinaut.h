/** @file omg_rabinaut.h Class RabinAutomaton */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2025 Yiheng Tang, Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef FAUDES_OMG_RABINAUT_H
#define FAUDES_OMG_RABINAUT_H

#include "corefaudes.h"
#include "omg_rabinacc.h"

namespace faudes {


/**
 * Generator with Rabin acceptance conditiom. 
 * 
 * @section SecRabinAut Overview
 * 
 * The TrGenerator is a variant of the TcGenerator to add an interface for a Rabin acceptance condition.
 * For convenience, the configuration with the minimum attributes is been typedef-ed as RabinAutomaton.
 *
 * Technically, the construct is based on the global attribute class faudes::RabinAcceptance
 * derived from faudes::AttributeVoid. Hence TrGenerator expects an event attribute template parameter
 * with the minimum interface defined in RabinAcceptance.
 *
 * The current implementation provides the absolute minimum member access, i.e., methods to get and
 * the acceptance condition. A future implementation may be more elaborate at this end.
 *
 *
 * @ingroup GeneratorClasses
 */

template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
    class FAUDES_TAPI TrGenerator : public TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr> {    
  public:


    /**
     * Creates an emtpy RabinAutomaton object 
     */
    TrGenerator(void);

    /** 
     * RabinAutomaton from a std Generator. Copy constructor 
     *
     * @param rOtherGen
     */
    TrGenerator(const vGenerator& rOtherGen);
        
    /** 
     * RabinAutomaton from RabinAutomaton. Copy constructor 
     *
     * @param rOtherGen
     */
    TrGenerator(const TrGenerator& rOtherGen);

    /**
     * Construct from file
     *
     * @param rFileName
     *   Filename
     *
     * @exception Exception
     *   If opening/reading fails an Exception object is thrown (id 1, 50, 51)
     */
    TrGenerator(const std::string& rFileName);

    /**
     * Construct on heap
     *
     * @return 
     *   new Generator 
     */
     TrGenerator* New(void) const;

    /**
     * Construct on stack
     *
     * @return 
     *   new Generator 
     */
     TrGenerator NewRGen(void) const;

    /**
     * Construct copy on heap
     *
     * @return 
     *   new Generator 
     */
     TrGenerator* Copy(void) const;

    /**
     * Type test.
     *
     * Uses C++ dynamic cast to test whether the specified object
     * casts to a RabinAutomaton.
     *
     * NOT IMPLEMENTED
     *
     * @param pOther
     *   poinetr to object to test
     *
     * @return 
     *   TpGenerator reference if dynamic cast succeeds, else NULL 
     */
     virtual const Type* Cast(const Type* pOther) const {
       return dynamic_cast< const TrGenerator* > (pOther); };


    /**
     * Assignment operator (uses Assign)
     *
     * Note: you must reimplement this operator in derived 
     * classes in order to handle internal pointers correctly
     *
     * @param rOtherGen
     *   Other generator
     */
     TrGenerator& operator= (const TrGenerator& rOtherGen);
  
    /**
     * Assignment method
     *
     * Note: you must reimplement this method in derived 
     * classes in order to handle internal pointers correctly
     *
     * @param rSource
     *   Other generator
     */
     virtual TrGenerator& Assign(const Type& rSource);
   
    /**
     * Set Rabin acceptance Condition
     *
     * @param rRabAcc
     *   Acceptance conditiomn to set
     *
     */
     void RabinAcceptance(const faudes::RabinAcceptance& rRabAcc);

    /**
     * Get Rabin acceptance condition
     *
     *
     */
     const faudes::RabinAcceptance&  RabinAcceptance(void) const;

    /**
     * Get Rabin acceotance condition
     *
     *
     */
     faudes::RabinAcceptance&  RabinAcceptance(void);

    /**
     * Produce graphical representation of this Rabin automaton with colored states.
     * R-states are shown as green double circles, I-states as red single circles.
     * Only considers the first Rabin pair.
     *
     * @param rFileName
     *   Output file name
     * @param rOutFormat  
     *   Graphics format, e.g. "png", "jpg", "svg" (optional, default from file extension)
     * @param rDotExec
     *   Path to graphviz dot executable (optional, default "dot")
     */
    void RabinGraphWrite(const std::string& rFileName, 
                         const std::string& rOutFormat = "", 
                         const std::string& rDotExec = "dot") const;

    /**
     * Write Rabin automaton in DOT format with colored states.
     * This is the helper function called by RabinGraphWrite().
     *
     * @param rFileName
     *   DOT file name to write
     */
    void RabinDotWrite(const std::string& rFileName) const;

 protected:

    /** need to reimplement to care about Additional members */
    void DoAssign(const TrGenerator& rSrc);



}; // end class TpGenerator

    
/** 
 * Convenience typedef for std prioritised generator 
 */
typedef TrGenerator<RabinAcceptance, AttributeVoid, AttributeCFlags, AttributeVoid> RabinAutomaton;



/*
***************************************************************************
***************************************************************************
***************************************************************************

Implementation pGenerator

***************************************************************************
***************************************************************************
***************************************************************************
*/

/* convenience access to relevant scopes */
#define THIS TrGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define BASE TcGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#define TEMP template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>


// TrGenerator(void)
TEMP THIS::TrGenerator(void) : BASE() {
  FD_DG("TrGenerator(" << this << ")::TrGenerator()");
}

// TrGenerator(rOtherGen)
TEMP THIS::TrGenerator(const TrGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("TrGenerator(" << this << ")::TrGenerator(rOtherGen)");
}

// TrGenerator(rOtherGen)
TEMP THIS::TrGenerator(const vGenerator& rOtherGen) : BASE(rOtherGen) {
  FD_DG("TrGenerator(" << this << ")::TrGenerator(rOtherGen)");
}

// TrGenerator(rFilename)
TEMP THIS::TrGenerator(const std::string& rFileName) : BASE(rFileName) {
  FD_DG("TrGenerator(" << this << ")::TrGenerator(rFilename) : done");
}

// full assign of matching type (not virtual)
TEMP void THIS::DoAssign(const TrGenerator& rSrc) {
  FD_DG("TrGenerator(" << this << ")::operator = " << &rOtherGen);
  // recursive call base, incl virtual clear  
  BASE::DoAssign(rSrc);
}

// operator=
TEMP THIS& THIS::operator= (const TrGenerator& rSrc) {
  FD_DG("TrGenerator(" << this << ")::operator = " << &rSrc);
  DoAssign(rSrc);
  return *this;
}

// copy from other faudes type
TEMP THIS& THIS::Assign(const Type& rSrc) {
  // bail out on match
  if(&rSrc==static_cast<const Type*>(this))
    return *this;
  // dot if we can
  const THIS* pgen=dynamic_cast<const THIS*>(&rSrc);
  if(pgen!=nullptr) {
    DoAssign(*pgen);
    return *this;
  }
  // pass on to base
  FD_DG("TrGenerator(" << this << ")::Assign([type] " << &rSrc << "): call base");
  BASE::Assign(rSrc);  
  return *this;
}

// New
TEMP THIS* THIS::New(void) const {
  // allocate
  THIS* res = new THIS;
  // fix base data
  res->EventSymbolTablep(BASE::mpEventSymbolTable);
  res->mStateNamesEnabled=BASE::mStateNamesEnabled;
  res->mReindexOnWrite=BASE::mReindexOnWrite;  
  return res;
}

// Copy
TEMP THIS* THIS::Copy(void) const {
  // allocate
  THIS* res = new THIS(*this);
  // done
  return res;
}

// NewPGen
TEMP THIS THIS::NewRGen(void) const {
  // call base (fixes by assignment constructor)
  THIS res= BASE::NewCGen();
  return res;
}

// Member access, set
TEMP void THIS::RabinAcceptance(const faudes::RabinAcceptance& rRabAcc) {
  BASE::GlobalAttribute(rRabAcc);
}

// Member access, get by ref
TEMP RabinAcceptance& THIS::RabinAcceptance(void) {
  return *BASE::GlobalAttributep();
}

// Member access, get by const ref
TEMP const RabinAcceptance& THIS::RabinAcceptance(void) const {
  return BASE::GlobalAttribute();
}

// RabinGraphWrite implementation
TEMP void THIS::RabinGraphWrite(const std::string& rFileName, 
                                const std::string& rOutFormat, 
                                const std::string& rDotExec) const {
  FD_DG("TrGenerator(" << this << ")::RabinGraphWrite(...): " << typeid(*this).name());
  
  // Generate temp dot input file
  std::string dotin = CreateTempFile();
  if(dotin == "") {
    std::stringstream errstr;
    errstr << "Exception opening temp file";
    throw Exception("TrGenerator::RabinGraphWrite", errstr.str(), 2);
  }
  
  try {
    RabinDotWrite(dotin);
  } catch (faudes::Exception& exception) {
    FileDelete(dotin);
    std::stringstream errstr;
    errstr << "Exception writing dot input file";
    throw Exception("TrGenerator::RabinGraphWrite", errstr.str(), 2);
  }
  
  try {
    faudes::ProcessDot(dotin, rFileName, rOutFormat, rDotExec);
  } catch (faudes::Exception& exception) {
    FileDelete(dotin);
    std::stringstream errstr;
    errstr << "Exception processing dot file";
    throw Exception("TrGenerator::RabinGraphWrite", errstr.str(), 3);
  }
  
  FileDelete(dotin);
}

// RabinDotWrite implementation
TEMP void THIS::RabinDotWrite(const std::string& rFileName) const {
  FD_DG("TrGenerator(" << this << ")::RabinDotWrite(" << rFileName << ")");
  
  if(BASE::ReindexOnWrite())
    BASE::SetMinStateIndexMap();
  
  StateSet::Iterator lit;
  TransSet::Iterator tit;
  
  // Get the first (and only) Rabin pair
  StateSet rStates, iStates;
  if(RabinAcceptance().Size() > 0) {
    const RabinPair& rabinPair = *RabinAcceptance().Begin();
    rStates = rabinPair.RSet();
    iStates = rabinPair.ISet();
  }
  
  try {
    std::ofstream stream;
    stream.exceptions(std::ios::badbit|std::ios::failbit);
    stream.open(rFileName.c_str());
    
    stream << "// dot output generated by libFAUDES TrGenerator" << std::endl;
    stream << "digraph \"" << BASE::Name() << "\" {" << std::endl;
    stream << "  rankdir=LR" << std::endl;
    stream << "  node [shape=circle];" << std::endl;
    stream << std::endl;
    
    // Initial states
    stream << "  // initial states" << std::endl;
    int i = 1;
    for (lit = BASE::InitStatesBegin(); lit != BASE::InitStatesEnd(); ++lit) {
      std::string xname = BASE::StateName(*lit);
      if(xname == "") xname = ToStringInteger(BASE::MinStateIndex(*lit));
      stream << "  dot_dummyinit_" << i << " [shape=none, label=\"\", width=\"0.0\", height=\"0.0\" ];" << std::endl;
      stream << "  dot_dummyinit_" << i << " -> \"" << xname << "\";" << std::endl; 
      i++;
    }
    stream << std::endl;
    
    // Handle overlap R∩I states first (most specific)
    StateSet overlap = rStates * iStates;  // intersection
    if(!overlap.Empty()) {
      stream << "  // R∩I-states (both conditions)" << std::endl;
      for (lit = overlap.Begin(); lit != overlap.End(); ++lit) {
        std::string xname = BASE::StateName(*lit);
        if(xname == "") xname = ToStringInteger(BASE::MinStateIndex(*lit));
        // Purple double circle with orange fill for states in both R and I
        stream << "  \"" << xname << "\" [shape=doublecircle, color=purple];" << std::endl;
      }
      stream << std::endl;
    }
    
    // R-states (green double circles, excluding overlap)
    stream << "  // R-states (Rabin acceptance - visited finitely often)" << std::endl;
    for (lit = rStates.Begin(); lit != rStates.End(); ++lit) {
      // Skip if already processed as overlap
      if(overlap.Exists(*lit)) continue;
      
      std::string xname = BASE::StateName(*lit);
      if(xname == "") xname = ToStringInteger(BASE::MinStateIndex(*lit));
      stream << "  \"" << xname << "\" [shape=doublecircle, color=green];" << std::endl;
    }
    stream << std::endl;
    
    // I-states (red single circles, excluding overlap and R-states)
    stream << "  // I-states (Rabin acceptance - visited infinitely often)" << std::endl;
    for (lit = iStates.Begin(); lit != iStates.End(); ++lit) {
      // Skip if already processed as R-state or overlap
      if(rStates.Exists(*lit)) continue;
      
      std::string xname = BASE::StateName(*lit);
      if(xname == "") xname = ToStringInteger(BASE::MinStateIndex(*lit));
      stream << "  \"" << xname << "\" [color=red];" << std::endl;
    }
    stream << std::endl;
    
    // Regular marked states (traditional double circle, not in R or I)
    stream << "  // regular marked states" << std::endl;
    for (lit = BASE::MarkedStatesBegin(); lit != BASE::MarkedStatesEnd(); ++lit) {
      if(rStates.Exists(*lit) || iStates.Exists(*lit)) continue;
      
      std::string xname = BASE::StateName(*lit);
      if(xname == "") xname = ToStringInteger(BASE::MinStateIndex(*lit));
      stream << "  \"" << xname << "\" [shape=doublecircle];" << std::endl;
    }
    stream << std::endl;
    
    // Rest of states
    stream << "  // rest of stateset" << std::endl;
    for (lit = BASE::StatesBegin(); lit != BASE::StatesEnd(); ++lit) {
      if (BASE::ExistsInitState(*lit) || 
          BASE::ExistsMarkedState(*lit) ||
          rStates.Exists(*lit) || 
          iStates.Exists(*lit)) {
        continue;
      }
      
      std::string xname = BASE::StateName(*lit);
      if(xname == "") xname = ToStringInteger(BASE::MinStateIndex(*lit));
      stream << "  \"" << xname << "\";" << std::endl;
    }
    stream << std::endl;
    
    // Transition relation
    stream << "  // transition relation" << std::endl;
    for(tit = BASE::TransRelBegin(); tit != BASE::TransRelEnd(); ++tit) {
      std::string x1name = BASE::StateName(tit->X1);
      if(x1name == "") x1name = ToStringInteger(BASE::MinStateIndex(tit->X1));
      std::string x2name = BASE::StateName(tit->X2);
      if(x2name == "") x2name = ToStringInteger(BASE::MinStateIndex(tit->X2));
      stream << "  \"" << x1name << "\" -> \"" << x2name
             << "\" [label=\"" << BASE::EventName(tit->Ev) << "\"];" << std::endl;
    }
    
    stream << "}" << std::endl;
    stream.close();
    
  } catch (std::ios::failure&) {
    throw Exception("TrGenerator::RabinDotWrite", 
                   "Exception opening/writing dotfile \""+rFileName+"\"", 2);
  }
  
  BASE::ClearMinStateIndexMap();
}

  
#undef TEMP
#undef BASE
#undef THIS



} // namespace faudes
#endif // _H