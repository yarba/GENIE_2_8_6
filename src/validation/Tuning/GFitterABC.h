//____________________________________________________________________________
/*!

\class     genie::tune::GFitterABC

\brief     Pure abstract base class (pABC) for GENIE fitters.

\author    Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
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

#ifndef _genie_fitter_abc_h_
#define _genie_fitter_abc_h_

#include <TString.h>

namespace genie {
namespace tune {

class GFitterABC
{
public:
   virtual ~GFitterABC();

   virtual bool       DoTheFit      (void)                   = 0; 
   virtual int        FitStatusCode (Option_t * option = "") = 0; 
   virtual double     FitTime       (void)                   = 0;
   virtual int        FitNTries     (void)                   = 0;
   virtual int        NDOF          (void)                   = 0;
   virtual int        NBinsInFit    (void)                   = 0;
   virtual int        NFitParams    (void)                   = 0;
   virtual void       SetTol        (double tol)             = 0;
   virtual void       SetStr        (int str   )             = 0;

protected:
   GFitterABC();
};

} // tune
} // genie

#endif

