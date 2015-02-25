#include <sstream>
#include <cassert>
#include <algorithm>

#include "RWProducerCCQE.h"

#include <TSystem.h>

#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCFormat.h"
#include "Ntuple/NtpMCTreeHeader.h"
#include "Ntuple/NtpMCEventRecord.h"
#include "Messenger/Messenger.h"
#include "Numerical/RandomGen.h"
// #include "PDG/PDGCodes.h"
// #include "PDG/PDGCodeList.h"

#include "Interaction/Interaction.h"

#include "ReWeight/GSystUncertainty.h"
// --> ??? #include "ReWeight/GReWeightUtils.h"

// Modules to calc weights/uncertainties
// by varying specific variables
//
// These ones right below is for CCQE (MaCCQE, etc.)
//
#include "ReWeight/GReWeightNuXSecCCQE.h"
#include "ReWeight/GReWeightNuXSecNCEL.h"
#include "ReWeight/GReWeightNuXSecCCRES.h"
#include "ReWeight/GReWeightNuXSecCCQEvec.h"

// I/O for re-weighting info 
//
#include "RWBranchDesc.h"
#include "RWRecord.h"

void RWProducerCCQE::DoInit()
{

   // FIXME !!!
   // This can expanded to hold a group of re-weighting tools
   // for the CCQE group of events
   
   fGReWeight.AdoptWghtCalc( "xsec_ccqe", new genie::rew::GReWeightNuXSecCCQE() );

   // Get GSystSet and include the (single) input systematic parameter
   // NOTE: in this case we use kXSecTwkDial_MaCCQE (for "MaCCQE")
   //
   genie::rew::GSystSet& syst = fGReWeight.Systematics();
   syst.Init( fParam2Tweak );

   // By default GReWeightNuXSecCCQE is in `NormAndMaShape' mode 
   // where Ma affects the shape of dsigma/dQ2 and a different param affects the normalization
   // If the input is MaCCQE, switch the weight calculator to `Ma' mode
   //
   genie::rew::GReWeightNuXSecCCQE* rwccqe = dynamic_cast<genie::rew::GReWeightNuXSecCCQE*>( fGReWeight.WghtCalc("xsec_ccqe") );  
   rwccqe->SetMode( genie::rew::GReWeightNuXSecCCQE::kModeMa );

   return;
   
}

bool RWProducerCCQE::DoIt( genie::NtpMCEventRecord* mcrec, RWRecord* rwrec )
{

   assert(mcrec);
   assert(rwrec);
   
   genie::EventRecord& evt = *(mcrec->event);
                
   //
   // Select events to be re-weighted
   //
   // First, check of neutrio(s) to conisder (by default, all of them are included)
   //
   if ( !(fNu2ReWeight.ExistsInPDGCodeList( evt.Probe()->Pdg() ) ) ) return false;
      
   // Now, specifically, check if it's QEL && WeakCC process
   // because that's what we want to reweight (MaCCQE).
   // Also skip charm events (although if those are quite rare)
   //
   genie::Interaction* interaction = evt.Summary();    
   const genie::ProcessInfo& prinfo = interaction->ProcInfo();   
   const genie::XclsTag&     xclsv  = interaction->ExclTag();
   bool accept = ( prinfo.IsQuasiElastic() && prinfo.IsWeakCC() && !xclsv.IsCharmEvent() );
   if ( !accept ) return false;

   genie::rew::GSystSet& syst = fGReWeight.Systematics();
          
   for ( unsigned int itwk=0; itwk<fTwkDials.size(); ++itwk )
   {
       syst.Set( fParam2Tweak, fTwkDials[itwk] );
       fGReWeight.Reconfigure();
       double wt = fGReWeight.CalcWeight(evt);
       rwrec->Insert( fTwkDials[itwk], wt );
   } 
          
   return true;

}
