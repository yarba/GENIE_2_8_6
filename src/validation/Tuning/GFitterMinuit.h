//____________________________________________________________________________
/*!

\class    genie::tune::GFitterMinuit

\brief    A minuit-based GENIE fitter

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

#ifndef _genie_fitter_minuit_h_
#define _genie_fitter_minuit_h_

#include <TMinuit.h>
#include <TString.h>

#include "validation/Tuning/GFitterABC.h"

namespace genie {
namespace tune {

class GFitInputs;

class GFitterMinuit: public GFitterABC
{
public:
   GFitterMinuit();
   GFitterMinuit(GFitInputs * inputs, Option_t * option = "minuit+migrad_err"); // option: "minuit+migrad_err", "minuit+hesse_err", "minuit+minos_err" 
   virtual ~GFitterMinuit();

   virtual bool       DoTheFit      (void);
   virtual int        FitStatusCode (Option_t * option = ""); // option: "migrad_min", "migrad_err", "hesse_err", "minos_err"
   virtual double     FitTime       (void)              { return fFitTime;                    }
   virtual int        FitNTries     (void)              { return fFitTries;                   }
   virtual int        NDOF          (void)              { return fNFitBins - fNFreeFitParams; }
   virtual int        NBinsInFit    (void)              { return fNFitBins;                   }
   virtual int        NFitParams    (void)              { return fNFreeFitParams;             }
   virtual void       SetTol        (double tol)        { fTol  = tol ;                       }
   virtual void       SetStr        (int str   )        { fStr  = str ;                       }
   
protected:

   void  Init    (void);
   void  Init    (GFitInputs * fit_inputs, Option_t * option = "migrad_err");
   bool  RunOnce (bool use_simplex);

   GFitInputs* fFitInputs;           ///< fit inputs
   TString     fOption;              ///< fit option
   int         fMigradMinStatusCode; ///< MIGRAD minimization status code
   int         fMigradErrStatusCode; ///< MIGRAD err status code
   int         fHesseErrStatusCode;  ///< HESSE err status code
   int         fMinosErrStatusCode;  ///< MINOS err status code
   double      fFitTime;             ///< time taken to do the fit
   int         fFitTries;            ///< number of tries taken to do the fit
   int         fNFitBins;            ///< number of bins in fit
   int         fNFreeFitParams;      ///< number of free params in fit
   bool        fCalcHesseErrors;     ///< calculate HESSE errors?
   bool        fCalcMinosErrors;     ///< calculate MINOS errors?
   double      fTol;                 ///< set tolerance
   int         fStr;
};

} // tune
} // genie

#endif

