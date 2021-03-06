//____________________________________________________________________________
/*!

\class    genie::tune::GFitModelABC

\brief    Pure abstract base class (pABC) for predictions of experimental
          observables plugged into the validation and tuning frameworks.

\author   Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
          University of Liverpool & STFC Rutherford Appleton Laboratory

          Adapted from VALOR oscillation fitting group code developed by
          C. Andreopoulos (Liverpool & STFC RAL), T. Dealtry (Oxford),
          S. Dennis (Warwick), L. Escudero (IFIC), N.Grant (Lancaster) and
          D.Sgalaberna (ETHZ).

\created  Feb 05, 2015

\cpright  Copyright (c) 2003-2015, GENIE Neutrino MC Generator Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
          or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#ifndef _genie_expt_model_abc_h_
#define _genie_expt_model_abc_h_

#include <string>
#include <iostream>

#include "validation/Tuning/GFitParamList.h"

class TObjArray;

namespace genie {
namespace tune {

class GExModelABC
{
public:
  virtual ~GExModelABC();

  // Identify model with a name (to be propagated in output files, etc)
  virtual string Name(void) = 0;

  // Read configuration data from file
  virtual bool Read(string file, Option_t * opt = 0) = 0;

  // Generate predictions for a given set of physics parameters
  virtual bool Generate (const GFitParamList & fp, Option_t * opt = 0) = 0;

  // Write all predictions to the input container or to the input file
  virtual bool Write (TObjArray* collection, const char * tag = "model", Option_t * write_opt = 0) = 0;
  virtual bool Write (const char * filename, const char * tag = "model", Option_t * write_opt = 0, Option_t * file_opt = 0) = 0;
  
  // Get penalty term for current systematics
  virtual double GetFitStatisticPenalty(Option_t * opt = 0) const = 0;

protected:
   GExModelABC();
};

} // tune
} // genie

#endif

