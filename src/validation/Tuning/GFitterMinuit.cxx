//________________________________________________________________________________________
/*
 Copyright (c) 2003-2015, GENIE Neutrino MC Generator Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
 University of Liverpool & STFC Rutherford Appleton Laboratory

 Adapted from VALOR oscillation fitting group code developed by
 C. Andreopoulos (Liverpool & STFC RAL), T. Dealtry (Oxford),
 S. Dennis (Warwick), L. Escudero (IFIC), N.Grant (Lancaster) and
 D.Sgalaberna (ETHZ).

 For the class documentation see the corresponding header file.

*/
//________________________________________________________________________________________

#define __show_fit_result__ 
#define __show_fit_steps__ 

#include <cassert> 
#include <cstdlib> 
#include <vector> 

#include <TMath.h> 
#include <TRandom3.h> 
#include <TVirtualFitter.h> 
#include <TString.h> 
#include <TFile.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <TMatrixDSym.h>

#include "Messenger/Messenger.h"
#include "validation/Tuning/GFitterMinuit.h" 
#include "validation/Tuning/GFitInputs.h" 
#include "validation/Tuning/GFitParamList.h" 
#include "validation/Tuning/GExDataABC.h" 

using namespace genie::tune; 

using std::vector;
   
//____________________________________________________________________________ 
GFitterMinuit::GFitterMinuit() :
GFitterABC()
{
  this->Init();
}
//____________________________________________________________________________ 
GFitterMinuit::GFitterMinuit(GFitInputs * fit_inputs, Option_t * option) :
GFitterABC()
{
  this->Init(fit_inputs, option);
}
//____________________________________________________________________________ 
GFitterMinuit::~GFitterMinuit() 
{

}
//____________________________________________________________________________ 
void GFitterMinuit::Init(void) 
{
  fFitInputs           = 0;
  fOption              = "";
  fNFreeFitParams      = 0;
  fNFitBins            = 0;
  fCalcHesseErrors     = false;
  fCalcMinosErrors     = false;
//fSimplex             = false;
  fTol                 = 0.01;
  fStr                 = 1;
  fMigradMinStatusCode = -999;
  fMigradErrStatusCode = -999;      
  fHesseErrStatusCode  = -999; 
  fMinosErrStatusCode  = -999; 
  fFitTime             = 0;
  fFitTries            = 0;
}
//____________________________________________________________________________ 
int GFitterMinuit::FitStatusCode (Option_t * option) 
{ 
  if(!option) return 9999;

  TString opt(option);

  if ( opt.Contains("migrad_min") ) return fMigradMinStatusCode;
  if ( opt.Contains("migrad_err") ) return fMigradErrStatusCode;
  if ( opt.Contains("hesse_err")  ) return fHesseErrStatusCode;
  if ( opt.Contains("minos_err")  ) return fMinosErrStatusCode;
  
  return -999;
}
//____________________________________________________________________________ 
void GFitterMinuit::Init(GFitInputs * fit_inputs, Option_t * option) 
{
  LOG("Minuit", pINFO) << "Fitter initialization...";

  this->Init();

  fFitInputs = fit_inputs;
  if(!fFitInputs) {
     LOG("Minuit", pFATAL) << "** Null fit inputs!";
     exit(1);
  }

  GFitParamList & fit_params = fFitInputs->Params();
  int param_list_size = fit_params.Size();

  LOG("Minuit", pINFO) << "Fit parameter list size = " << param_list_size;

  fOption = option;

  // calc HESSE or MINOS errors after fit
  fCalcHesseErrors  = fOption.Contains("hesse");
  fCalcMinosErrors  = fOption.Contains("minos");

  if(fOption.Contains("minuit")) {
    // get fitter
    TVirtualFitter::SetDefaultFitter("Minuit");
    TVirtualFitter * fitter = TVirtualFitter::Fitter(0,param_list_size);
    
    // set print level
    Double_t arglist[5];
    arglist[0] = -1;
    fitter->ExecuteCommand("SET PRINT",arglist,1);
    
    // set params
    fit_params.SetTo(fitter);
    
    // set minimization function
    fitter->SetFCN(FitFunc);            
  }

/*
  if(fOption.Contains("simplex")) {
    TVirtualFitter::SetDefaultFitter("Simplex");
    TVirtualFitter * fitter = TVirtualFitter::Fitter(0,param_list_size);
    
    // set print level              
    Double_t arglist[5];
    arglist[0] = -1;
    fitter->ExecuteCommand("SET PRINT",arglist,1);
    
    // set params                
    fit_params.SetTo(fitter);
    
    // set minimization function     
    fitter->SetFCN(FitFunc);
    fSimplex = true;
  }
*/

  // calculate degrees of freedom  
  fNFitBins       = fit_inputs->Data().NumOfBins();
  fNFreeFitParams = fit_params.GetNumberFreeParameters();
  
  assert(fNFitBins > 0);
  assert(fNFreeFitParams >= 0);

  LOG("Minuit", pINFO) 
       << "NDoF = " << this->NDOF()
       << " (Number of bins = " << this->NBinsInFit()
       << ", Number of free parameters = " << this->NFitParams() << ")";
  
}
//____________________________________________________________________________ 
bool GFitterMinuit::DoTheFit(void) 
{
  // start clock
  TStopwatch w;
  w.Start();

  fFitTime  = 0;
  fFitTries = -1;
  
  // Initialise and get ready.
  // set params
  GFitParamList & fit_params = fFitInputs->Params();
  int param_list_size = fit_params.Size();
  TVirtualFitter * fitter = TVirtualFitter::Fitter(0,param_list_size);
  // set print level
  Double_t arglist[5];
  arglist[0] = -1;
  fitter->ExecuteCommand("SET PRINT",arglist,1);
  
  // set params
  fit_params.SetTo(fitter);
  // set minimization function
  fitter->SetFCN(FitFunc);         
    
  // Done.
  // Reset fit time, bestfit statistic and status codes in persistable GFitParamList
  fFitInputs->Params().SetFitTime(0);
  fFitInputs->Params().SetBestFitStatistic(0);
  fFitInputs->Params().ClearStatusCodes();

  // Run the fitter once - we have the option here to check the status code
  // and if the fitter failed, re-seed the fitter and try again
  bool ok = this->RunOnce(false);

  // 4: migrad not converged
  fFitTries = 1;

  if(fMigradMinStatusCode == 4) {
    LOG("Minuit", pWARN) << "Fits still failed; trying SIMPLEX";
    this->RunOnce(true);
    fFitTries = 99;
  }  

  // stop clock
  w.Stop();
  fFitTime = w.CpuTime();

  LOG("Minuit", pINFO) 
    << "Fit performed in " << fFitTime 
    << " sec after " << fFitTries << " attempts";
  LOG("Minuit", pINFO) 
    << "Best-fit statistic: " << fFitInputs->Params().GetBestFitStatistic();

  // copy fit time and bestfit statistic and status codes 
  // to persistable FitParamList obj

  fFitInputs->Params().SetFitTime (fFitTime);
  fFitInputs->Params().SetFitTries(fFitTries);
  fFitInputs->Params().SetBestFitStatistic(fFitInputs->Params().GetBestFitStatistic());

  fFitInputs->Params().SetStatusCode("migrad_min", fMigradMinStatusCode);
  fFitInputs->Params().SetStatusCode("migrad_err", fMigradErrStatusCode);

  if(fCalcHesseErrors) {
    fFitInputs->Params().SetStatusCode("hesse_err", fHesseErrStatusCode);
  }
  if(fCalcMinosErrors) {
    fFitInputs->Params().SetStatusCode("minos_err", fMinosErrStatusCode);
  }
  return ok;
}
//____________________________________________________________________________ 
bool GFitterMinuit::RunOnce(bool use_simplex) 
{
  fHesseErrStatusCode = -1;
  double fcntemp;

  // Get the fitter
#ifdef __show_fit_steps__
  LOG("Minuit", pINFO) << "Getting the fitter...";
#endif
  TVirtualFitter * fitter = TVirtualFitter::GetFitter();

  Double_t arg[5];
  arg[0] = fStr;
  fitter->ExecuteCommand("SET STRategy",arg,1);    

  // Pass fit inputs (data, pdf) to MINUIT
#ifdef __show_fit_steps__
  LOG("Minuit", pINFO) << "Passing fit inputs (data & pdf)...";
#endif
  gMinuit->SetObjectFit(fFitInputs);

  // Run MINUIT's minimization step
#ifdef __show_fit_steps__
  LOG("Minuit", pINFO) << "Running MIGRAD...";
#endif
  Double_t arglist[100];
  arglist[0] = 50000; // num of func calls
  arglist[1] = fTol;  // tolerance
  if(!use_simplex) {    
    fMigradMinStatusCode = fitter->ExecuteCommand("MIGRAD",arglist, 2);    
  }//!_use_simplex
  //SIMPLEX
  else {
    arglist[1] = 0.1; // higher tolerance
    fMigradMinStatusCode = fitter->ExecuteCommand("SIMPLEX",arglist, 2);
  }  
  //return on not converged
  if(fMigradMinStatusCode == 4 && !use_simplex)
    return false;
  else if(fMigradMinStatusCode == 4 && use_simplex) {
    //copy current to best fit, to stop horrible crashes
    fFitInputs->Params().CopyCurrToBestFitValues();
    return false;
  }
  
  int num_of_params = fFitInputs->Params().Size();
  
  // Found best fit! Copy current --> best-fit values
  for(int ip = 0; ip < num_of_params; ip++) {
    double value, error;
    gMinuit->GetParameter(ip, value, error);
    GSyst_t p = fFitInputs->Params().WhichParam(ip);
    fFitInputs->Params().Update(p, value);
  }
  fFitInputs->Params().CopyCurrToBestFitValues();

  // Print fit result
#ifdef __show_fit_result__
#ifdef __show_fit_steps__
  LOG("Minuit", pINFO) << "Printing MIGRAD fit results...";
#endif
  fitter->ExecuteCommand("SHOW FCNVALUE",&fcntemp,0);
  Double_t aminn = 0;
  fitter->PrintResults(3,aminn);
#endif

  TMatrixDSym cov(num_of_params);
  for(int i=0; i<num_of_params; i++) {
    for(int j=0; j<num_of_params; j++) {
        cov[i][j] = fitter->GetCovarianceMatrixElement(i,j);
    }
  }
  fFitInputs->Params().SetCovMatrix(cov);

  double unfilled = -999;

  // Error vectors
  vector<double> migrad_err     (num_of_params, unfilled);
  vector<double> hesse_err      (num_of_params, unfilled);
  vector<double> minos_err_minus(num_of_params, unfilled);
  vector<double> minos_err_plus (num_of_params, unfilled);
  
  // Fill the MIGRAD errors
  for (int ip = 0 ; ip < num_of_params ; ip++) {
    migrad_err[ip] = fitter->GetParError(ip);
  }
  fFitInputs->Params().SetMigradErrors(migrad_err);

  // Get the error matrix status
  double ha(0.0);     // dummy vars
  double edm, errdef; // dummy vars
  int nvpar, nparx;   // dummy vars
  int errMatrixStatus = fitter->GetStats(ha, edm, errdef, nvpar, nparx);
  fMigradErrStatusCode = errMatrixStatus;

  // Calculate HESSE or MINOS errors
  if(fCalcHesseErrors) {
#ifdef __show_fit_steps__
    LOG("Minuit", pINFO) << "Running HESSE...";
#endif
    fHesseErrStatusCode = fitter->ExecuteCommand("HESSE",arglist, 1);
    
    // Fill the HESSE errors
    for (int ip = 0 ; ip < num_of_params ; ip++) {
      hesse_err[ip] = fitter->GetParError(ip);
    }
    fFitInputs->Params().SetHesseErrors (hesse_err);
  }
  
  //else 
  if(fCalcMinosErrors) {
#ifdef __show_fit_steps__
    LOG("Minuit", pINFO) << "Running MINOS...";
#endif
    fMinosErrStatusCode = fitter->ExecuteCommand("MINOS",arglist, 1);
  }
  // Get error matrix status
#ifdef __show_fit_steps__
  LOG("Minuit", pINFO) << "Getting error matrix status...";
#endif
  errMatrixStatus = fitter->GetStats(ha, edm, errdef, nvpar, nparx);

  // Get errors
#ifdef __show_fit_steps__
  LOG("Minuit", pINFO) << "Getting errors...";
#endif

  if(fCalcMinosErrors) {
    for(int ip = 0; ip < num_of_params; ip++) {
      double glob_corr, parerr; // dummy vars
        fitter->GetErrors(ip, minos_err_plus[ip], minos_err_minus[ip],parerr,glob_corr);
    }
    fFitInputs->Params().SetMinosErrors (minos_err_plus, minos_err_minus);
  }
  
  // Print fit result
  if(fCalcHesseErrors) {
#ifdef __show_fit_result__
#ifdef __show_fit_steps__
    LOG("Minuit", pINFO) << "Printing HESSE fit results...";
#endif
    Double_t amin = 0;
    fitter->PrintResults(3,amin);
  //fitter->PrintResults(1,amin); // prints limits
  }
  LOG("Minuit", pINFO) << "Final error matrix status = " << fMigradErrStatusCode; 
#endif

#ifdef __show_fit_steps__
  LOG("Minuit", pINFO) << "Done";
#endif

  return true;//(fMigradErrStatusCode == 3);
}
//____________________________________________________________________________ 
