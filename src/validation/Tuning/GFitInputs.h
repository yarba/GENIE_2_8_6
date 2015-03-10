//____________________________________________________________________________
/*!

\class    genie::tune::GFitInputs

\brief    Bundles together all fitter inputs (data, model, params)

          * For Minuit-based fits:
          This TObject-derived class can be passed to the Minuit fitting 
          function via TMinuit::Set/GetObjectFit()
          * For Minuit2-based fits:
          This FCNBase-derived class can be passed to Minuit2 and implements
          the required double operator() (const std::vector<double>& x) const
          * For GSL-based fits:
          Returns the appropriate functors for GSL algorithms

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

#ifndef _genie_fit_inputs_h_
#define _genie_fit_inputs_h_

/*
#include <Minuit2/FCNBase.h>
#include <Math/IFunction.h>
#include <Math/FitMethodFunction.h>
#include <HFitInterface.h>
*/

#include "validation/Tuning/GFitParamList.h"

namespace genie {
namespace tune {
  
class GExDataABC;
class GExModelABC; 
class GFitParamList;

class GFitInputs: /* public ROOT::Minuit2::FCNBase, */ public TObject
{
public :
  GFitInputs(GExDataABC * data, GExModelABC * model, GFitParamList * params);
 ~GFitInputs();
  
  GExDataABC &    Data   (void);
  GExModelABC &   Model  (void);
  GFitParamList & Params (void);

  void UpdateParamList(GFitParamList * fp);
  
  double Statistic(Option_t * stat_opt = "likelihood-ratio") const;

/*
  // Minuit2
  //
  double Up          (void) const    { return fErrDef;   }
  double ErrorDef    (void) const    { return Up();   }
  void   SetErrorDef (double errdef) { fErrDef = errdef; } 
  double operator() (const std::vector<double>& x) const; // Minuit2: Returns chi^2
*/

/*  
  // GSL
  //
  ROOT::Math::IMultiGenFunction * AsFunctor          (void) { return new FitFunctor(this); }
  ROOT::Math::FitMethodFunction * AsFitMethodFunction(void) { return new FitMethodFunc(this->GetNDim(),this->GetNPoints(),this); }
*/
  
  int GetNDim    (void) const {return fNDim;   }
  int GetNPoints (void) const {return fNPoints;}

  double ChisquarePenaltyTerm(void) const { return 0.; } // JVY, 3/10/2015: tmp hack to make build sensibly

private:
  
  GExDataABC *             fData;
  mutable GExModelABC *    fModel;
  mutable GFitParamList *  fParams;
  string                   fOpt;
//  int                      fMode;
//  double                   fErrDef;
//  bool                     fAllBANFFFit;
//  bool                     fAll1ReEff;
//  bool                     fGoodnessOfFit;
  int                      fNPoints;
  int                      fNDim;
  
  ClassDef(GFitInputs, 1)    
};

//............................................................................  
//
// MINUIT, GSL fit functions
//

void FitFunc (Int_t &, Double_t *, Double_t &f, Double_t *par, Int_t iflag);

/*
//............................................................................
class FitFunctor: public ROOT::Math::IMultiGenFunction
{
public:
  FitFunctor(FitInputs * inputs);
  ~FitFunctor();
  double operator() (const double * x) const;
  double DoEval(const double * x) const;
  //ROOT::Math::IBaseFunctionMultiDim * Clone(void) const;
  ROOT::Math::IMultiGenFunction * Clone(void) const;
  unsigned int	NDim(void) const;
 private:
  FitInputs * fFitInputs;
};
//............................................................................
class FitMethodFunc : public ROOT::Math::FitMethodFunction 
{
 public:
   FitMethodFunc( int dim, int npoints,  FitInputs * inputs) :
      ROOT::Math::FitMethodFunction(dim,npoints), fFitInputs(inputs) {}
  ~FitMethodFunc();
  double operator() (const double * x) const;
  ROOT::Math::IMultiGenFunction * Clone(void) const{ return new FitMethodFunc(*this);  }  
  double DataElement(const double* x, unsigned int i, double* g = 0) const;
  virtual ROOT::Math::BasicFitMethodFunction<ROOT::Math::IBaseFunctionMultiDim>::Type_t Type(void) const; 
 private:
  mutable FitInputs * fFitInputs;
  virtual double DoEval(const double * x) const;  
};
*/

} // tune
} // genie

#endif

