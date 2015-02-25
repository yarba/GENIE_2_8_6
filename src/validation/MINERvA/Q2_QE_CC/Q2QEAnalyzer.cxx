
#include "TROOT.h"

#include "Q2QEAnalyzer.h"

#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCEventRecord.h"

#include "Interaction/Interaction.h"
#include "Interaction/Target.h"
#include "Conventions/Units.h"
#include "PDG/PDGCodes.h"

#include "RWRecord.h"

using namespace genie;
using namespace PlotUtils;

void Q2QEAnalyzer::Reset()
{

   fXSec = 0.;
   fSimHisto->Reset();

   return;

}

void Q2QEAnalyzer::EndOfRun()
{

   // finalize cross section
   //
   // bear in kind that the xsec as of this analyzer has been calculated
   // for a specific group of events: QE && CC && !charm
   // for this reason it needs to be scaled with the # events that satisfy these criteria 
   // ( not the total number of events in the sample )
   // in addition, the xsec has been calculated ** per neutron **
   // (as in MINERvA paper PRL 101, 022502 (2013)
   //   
   int NEntries   = fSimHisto->GetEntries();
   fXSec /= (double)NEntries;
      
   fEndOfRunApplied=true;

   return;
}

void Q2QEAnalyzer::DoIt( NtpMCEventRecord* mcrec, RWRecord* rwrec )
{

   // double check
   //
   assert(fSimHisto);
   
   EventRecord&   event      = *(mcrec->event); 
   Interaction* interaction = event.Summary(); 
   
   // Check if it's QEL && WeakCC process
   // because that's what MINERvA has measured/published
   // Also skip charm events (although if those are quite rare)
   //
   const ProcessInfo& prinfo = interaction->ProcInfo();   
   const XclsTag&     xclsv  = interaction->ExclTag();
   bool accept = ( prinfo.IsQuasiElastic() && prinfo.IsWeakCC() && !xclsv.IsCharmEvent() );
   if ( !accept ) return ;
    
   Target* target = (interaction->InitState()).TgtPtr(); 
   assert(target);
   
   double diffxsec = event.XSec();
   diffxsec *= (1E+38/units::cm2); // for more info on unites - see GENIE manual, section 6.7 (p.122)
   
   if ( fInteractionType.GetTgtNucleonPDG() == kPdgNeutron || fInteractionType.GetTgtNucleonPDG() == kPdgTgtFreeN )
   {
      if ( target->N() > 0 )
      { 
         diffxsec /= target->N();
      }
   }
   else if ( fInteractionType.GetTgtNucleonPDG() == kPdgProton || fInteractionType.GetTgtNucleonPDG() == kPdgTgtFreeP )
   {
      diffxsec /= target->Z();
   }
   fXSec += diffxsec;
   
   // Fill up the histo with the Q2 value as it's available with the event info
   // NOTE: as an alternative, one can also calculate the Q2 value "by hands"
   //       - see GENIE manual, p.108, for an example of doing such calculations
   //
   const double val = (interaction->Kine()).Q2(true);
   fSimHisto->Fill( val );
   
   // check if (no) re-weighting...
   //
   if ( !rwrec ) return;
   if ( rwrec->GetNumOfRWResults() <= 0 ) return;
   
   std::vector<double> wt;
   int nres = rwrec->GetNumOfRWResults();
   wt.reserve( nres );
   wt.clear();

   for ( int ir=0; ir<nres; ++ir )
   {
      wt.push_back(  rwrec->GetWeight( ir ) );
   }
      
   // init (if needs be) and fill up the vertical band 
   // with the weights coming from GENIE reweighting machinery
   //
   if ( !fSimHisto->HasVertErrorBand("Reweight-Mass-CCQE") )
   {
      // Note-1: tmp work around a RooMUHistos issue (suggested by Brian Tice of ANL )
      // force err.band creation in the same directory as the CV histo
      //
      // Note-2: The issue is fixed as of 10/28/2014
      //
// -->      TString workdir = gDirectory->GetPath();
// -->      fSimHisto->GetDirectory()->cd();
      //
      fSimHisto->AddVertErrorBand( "Reweight-Mass-CCQE", wt.size() );
      fSimHisto->GetVertErrorBand( "Reweight-Mass-CCQE" )->Reset();
      //
      // set back to whatever directory the app was in
      //
// -->      gDirectory->cd(workdir);
   }
   fSimHisto->FillVertErrorBand( "Reweight-Mass-CCQE", val, wt );
   
   return;
   
}
