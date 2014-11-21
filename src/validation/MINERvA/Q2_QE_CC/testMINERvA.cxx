
#include <iostream>
#include <sstream>

#include <string>
#include <vector>

#include "Messenger/Messenger.h"

#include "RunConfig.h"
#include "ExpData.h"
#include "MCAnalysis.h"
#include "MINERvADataMC.h"


// provisions for future development
//
// #include "Utils/GSimFiles.h"
// #include "Utils/CmdLnArgParser.h"

using namespace PlotUtils;
using namespace genie; // for the GENIE's LOG Messager... 

//
// current usage pattern (example):
//
// ./tstMINERvA -d MINERvA_expdata_input.xml
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

// --> provisions for future development
//
//   GSimFiles* gsimreader = new GSimFiles( false, 10 ); // no chain; max 10 models (i.e. versions)
//   bool ok = gsimreader->LoadFromFile( "./gntp.1.ghep.root" );
      
   //
   // clear everything out  before startingg run(s)
   //
   mcanalysis->Reset();

   // analyze MC sample(s), with or without re-weighting (omit the 2nd arg if no re-weighting)
   //
   // NOTE: there'll be run-time configuration in the future
   //
//   mcanalysis->Analyze( "./genie-samples-reweight/gntp.1.ghep.root", "./genie-samples-reweight/nu-CH-weights-MaCCQE.root" );
//   mcanalysis->Analyze( "./genie-samples-reweight/gntp.2.ghep.root", "./genie-samples-reweight/nubar-CH-weights-MaCCQE.root" );
   mcanalysis->Analyze( "/data/g4/tmp-genie-samples/nu-Hydrocarbon/100K/gntp.1.ghep.root", 
                        "/data/g4/tmp-genie-samples/nu-Hydrocarbon/100K/nu-CH-weights-MaCCQE.root");
   mcanalysis->Analyze( "/data/g4/tmp-genie-samples/nubar-Hydrocarbon/100K/gntp.2.ghep.root", 
                        "/data/g4/tmp-genie-samples/nubar-Hydrocarbon/100K/nubar-CH-weights-MaCCQE.root");

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

