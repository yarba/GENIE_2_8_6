
#include <iostream>
#include <sstream>

#include <string>
#include <vector>

#include "Messenger/Messenger.h"

#include "validation/MINERvA/Common/RunConfig.h"
#include "validation/MINERvA/Common/ExpData.h"
#include "MCAnalysis.h"
#include "MINERvADataMC.h"

using namespace PlotUtils;
using namespace genie; // for the GENIE's LOG Messager... 

//
// current usage pattern (example):
//
// ./gvld_MINERvA_Q2_QE_CC -g example_mc_input.xml-d MINERvA_expdata_input.xml
//

int main( int argc, char ** argv )
{
   
   RunConfig* run      = new RunConfig( argc, argv );
         
   // instantiate MC analysis
   //
   MCAnalysis* mcanalysis = new MCAnalysis();
   
   // based on exp.data, ass dataset-specific analyzers to the analysis
   //
   // NOTE: if no matching analyzer for a given exp.observable, 
   //       no analyzer will be added; a warning will be printed out
   //
   for ( int i=0; i<run->GetExpData()->GetNDataSets(); ++i )
   {
      const ExpDataSet* dset   = run->GetExpData()->GetExpDataSet( i );
      mcanalysis->AddMCAnalyzer( dset );
   }
      
   //
   // clear everything out  before startingg run(s)
   //
   mcanalysis->Reset();

   // analyze MC sample(s), with or without re-weighting (omit the 2nd arg if no re-weighting)
   //
   // NOTE: there'll be run-time configuration in the future
   //
   const GSimFiles* mcfiles = run->GetSimFilesHandler();
   int imodel = 0 ; // this is a hack ! there may be several models(versions) of GENIE !!! (regression test)
   //
   for ( unsigned int imc=0; imc<mcfiles->EvtFileNames(imodel).size(); ++imc )
   {
      mcanalysis->Analyze( mcfiles->EvtFileNames(imodel)[imc] );
   }

   //
   // finalize MC results
   //
   mcanalysis->EndOfRun(); 
   
   //
   // final Data to MC comparison, plots, chi2 tests, etc.
   //
   MINERvADataMC* datamc = new MINERvADataMC();   
   datamc->FinalizeResults( run->GetExpData(), mcanalysis );

   delete datamc;
   delete mcanalysis;
   delete run;

   // use GENIE message logger for (various) printouts
   //
   LOG("gvldtest", pNOTICE) << "Finishing up ! ";   
   
   return 0;

}

