
#include "ResultsDataMC.h"

#include "ExpData.h"
#include "MCAnalysis.h"

void ResultsDataMC::FinalizeResults( const ExpData* data, const MCAnalysis* mcanalysis )
{

   for ( int i=0; i<data->GetNDataSets(); ++i )
   {
      
      const ExpDataSet*   dset     = data->GetExpDataSet( i );
      const BaseAnalyzer* analyzer = mcanalysis->GetMCAnalyzer( dset );
          
      if ( !analyzer ) continue; // skip if no matching analyzer (need to print a warning message as well !)
      if ( analyzer->IsUnphysical() ) continue; // skip empty histo 
                                                // there maybe some because the # of datasets
						// doesn't necessarily matches the # of MC samples   
      
      DoIt( dset, analyzer );
   }

   return;

}
