#include <string>
#include <sstream>
//#include <cassert>

#include "RWRunConfig.h"
#include "RWProducerCCQE.h"

int main( int argc, char** argv )
{

   RWRunConfig* run = new RWRunConfig( argc, argv );
      
   RWProducerCCQE* rwprod = new RWProducerCCQE( run->GetParam2Tweak(), run->GetTweaks(), "UPDATE" );
   rwprod->UpdateNu2ReWeightList( run->GetListOfNu2ReWeight() );
   
   int NM = run->GetSimFilesHandler()->NModels();
   for ( int im=0; im<NM; ++im )
   {
      std::vector<std::string>& samples = run->GetSimFilesHandler()->EvtFileNames( im );
      for ( unsigned int is=0; is<samples.size(); ++is )
      {
         rwprod->Reset();
	 rwprod->ReWeight( samples[is] );
      }
   }
   
   delete rwprod;
   delete run;
      
   return 0;

}
