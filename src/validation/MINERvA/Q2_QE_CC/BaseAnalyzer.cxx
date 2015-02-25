
#include "BaseAnalyzer.h"

#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCEventRecord.h"

#include "Interaction/Interaction.h"
#include "Interaction/Target.h"

using namespace genie;
using namespace PlotUtils;

BaseAnalyzer::BaseAnalyzer()
   : fObservable(""), fXSec(0.), fSimHisto(0), fEndOfRunApplied(false)
{
}

BaseAnalyzer::~BaseAnalyzer()
{

   if ( fSimHisto ) delete fSimHisto;

}

void BaseAnalyzer::Init( const IntType& type, const std::string& oname, const TH1D& histo )
{

   SetInteraction( type );
   
   int nxbins = histo.GetNbinsX();
   double* xbins = new double[nxbins+1];
   for ( int i=0; i<nxbins; ++i )
   {
      xbins[i] = histo.GetBinLowEdge(i+1);
      xbins[i+1] = xbins[i] + histo.GetBinWidth(i+1);
   }
   
   // instantiate SimHisto with variable bin width
   // try to make its name unique
   // title is empty (at least, for now)
   //
   std::string hname = "SimHisto";
   std::string prj = fInteractionType.GetProjectileName();
   std::string tg  = fInteractionType.GetTgtNucleonName();
   hname += ( "_" + prj + "_" + tg + "_" + fObservable );
   fSimHisto = new MUH1D( hname.c_str(), "", nxbins, xbins );
   fSimHisto->GetXaxis()->SetTitle( fObservable.c_str() );
   fSimHisto->GetXaxis()->SetTitleFont(62);
   fSimHisto->GetXaxis()->CenterTitle();
   
   delete [] xbins;
   
   return;

}

void BaseAnalyzer::Analyze( NtpMCEventRecord* mcrec, RWRecord* rwrec )
{
 
   EventRecord&   event      = *(mcrec->event); 
   GHepParticle* probe = event.Probe();
   assert(probe);
   Interaction* interaction = event.Summary();  
   Target* target = (interaction->InitState()).TgtPtr(); 
   assert(target);
   
   if ( !fInteractionType.CheckInteraction( probe->Pdg(), target->HitNucPdg() ) )
   {
      return; // skip interactions that are inconsistent with initial settings
   }
   
   // actual work is done here
   // (this method is purely virtual)
   //
   DoIt( mcrec, rwrec );
   
   return;

}

bool BaseAnalyzer::IsUnphysical() const
{

   if ( !fSimHisto ) return true;
   
   if ( fSimHisto->GetEntries() <= 0 ) return true;
   
   return false;

}

MUH1D BaseAnalyzer::GetSimHistoFluxNormalized() const
{

   // FIXME !!!
   // need to check on fEndOfRunApplied 
   // to make sure that XSec is properly calculated!!!
   
   MUH1D h( *fSimHisto );

   int NEntries = fSimHisto->GetEntries();   
   //
   // since it's "flux-normalized", it needs to account 
   // for the *total* number of entries (incl. overflow)
   //
   double Scale = fXSec / (double)NEntries;
   h.Scale( Scale, "width" );
   
   return h; 


}
MUH1D BaseAnalyzer::GetSimHistoAsShape() const
{

   MUH1D h( *fSimHisto );
   
   double Scale = 1. / fSimHisto->Integral();
   h.Scale( Scale );
   
   return h; 


}
