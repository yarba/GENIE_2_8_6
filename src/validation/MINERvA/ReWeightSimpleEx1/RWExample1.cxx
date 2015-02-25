#include <string>
#include <sstream>
#include <cassert>

#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TArrayF.h>

#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCFormat.h"
#include "Ntuple/NtpMCTreeHeader.h"
#include "Ntuple/NtpMCEventRecord.h"
#include "Messenger/Messenger.h"
#include "Numerical/RandomGen.h"
#include "PDG/PDGCodes.h"
#include "PDG/PDGCodeList.h"

#include "Interaction/Interaction.h"
// --> ??? #include "Interaction/Target.h"
// --> ??? #include "Conventions/Units.h"

#include "ReWeight/GReWeightI.h"
#include "ReWeight/GSystSet.h"
#include "ReWeight/GSyst.h"

#include "ReWeight/GReWeight.h"
#include "ReWeight/GSystUncertainty.h"
// --> ??? #include "ReWeight/GReWeightUtils.h"

#include "ReWeight/GReWeightNuXSecCCQE.h"
/* -- for future...
#include "ReWeight/GReWeightNuXSecNCEL.h"
#include "ReWeight/GReWeightNuXSecCCRES.h"
#include "ReWeight/GReWeightNuXSecCOH.h"
#include "ReWeight/GReWeightNonResonanceBkg.h"
#include "ReWeight/GReWeightFGM.h"
#include "ReWeight/GReWeightDISNuclMod.h"
#include "ReWeight/GReWeightResonanceDecay.h"
#include "ReWeight/GReWeightFZone.h"
#include "ReWeight/GReWeightINuke.h"
#include "ReWeight/GReWeightAGKY.h"
#include "ReWeight/GReWeightNuXSecCCQEvec.h"
#include "ReWeight/GReWeightNuXSecNCRES.h"  
#include "ReWeight/GReWeightNuXSecDIS.h"  
*/

#include "RWBranchDesc.h"
#include "RWRecord.h"
  
// --> #include "Utils/AppInit.h"
// --> #include "Utils/RunOpt.h"
// --> #include "Utils/CmdLnArgParser.h"

int main( int argc, char ** argv )
{

   // Prepare output
   //
//   TFile* ofile = new TFile( "test_rw.root", "RECREATE" );
   TFile* ofile = new TFile( "test_rw.root", "UPDATE" );
   //
   // Create a new tree for RW objects
   //
   TTree* rwtree = new TTree( "reweighting", "GENIE weights tree" );
   rwtree->SetAutoSave( 200000000 );  // autosave when 0.2 Gbyte written 
                                      // - it's the same for "gtree" but I need to double check 
				      // how to *get* autosave from "gtree" 
   RWRecord* rwrec = 0;
   TTree::SetBranchStyle(1);
   std::string brname = genie::rew::GSyst::AsString(genie::rew::kXSecTwkDial_MaCCQE); 
   TBranch* rwbr = rwtree->Branch( brname.c_str(), "RWRecord", &rwrec, 32000, 1 ); // FIXME !!! also check more "sophisticated" options  
   assert(rwbr); 
   rwbr->SetAutoDelete(kFALSE); 

   // Create a GReWeight object and add to it a set of weight calculators
   //
   genie::rew::GReWeight rw;
   
   // Add weight calculator for MaCCQE  
   // NOTE: will add other weight calculators later
   //
   rw.AdoptWghtCalc( "xsec_ccqe", new genie::rew::GReWeightNuXSecCCQE() );

   // Get GSystSet and include the (single) input systematic parameter
   // NOTE: in this case we use kXSecTwkDial_MaCCQE (for "MaCCQE")
   //
   genie::rew::GSystSet& syst = rw.Systematics();
   // syst.Init(gOptSyst);
   syst.Init( genie::rew::kXSecTwkDial_MaCCQE );
   
   // By default GReWeightNuXSecCCQE is in `NormAndMaShape' mode 
   // where Ma affects the shape of dsigma/dQ2 and a different param affects the normalization
   // If the input is MaCCQE, switch the weight calculator to `Ma' mode
   //
   genie::rew::GReWeightNuXSecCCQE* rwccqe = dynamic_cast<genie::rew::GReWeightNuXSecCCQE*>( rw.WghtCalc("xsec_ccqe") );  
   rwccqe->SetMode( genie::rew::GReWeightNuXSecCCQE::kModeMa );

   // MaCCQE=0.99 has been extracted under gdb from GReWeightNuXSecCCQE class (member data fMaDef);
   // in principle, it depends on the physics model - in the case of GReWeightNuXSecCCQE the model 
   // is based on the "LwlynSmithQELCCPXSec" algorithm (see GReWeightNuXSecCCQE::Init() method)
   //
   // Sigma's (+/-) can be extracted from GSystUncertainty
   //
   genie::rew::GSystUncertainty* syser = genie::rew::GSystUncertainty::Instance();
   double sigpls = syser->OneSigmaErr( genie::rew::kXSecTwkDial_MaCCQE,  1 );
   double sigmin = syser->OneSigmaErr( genie::rew::kXSecTwkDial_MaCCQE, -1 );
   //
   rwtree->GetUserInfo()->Add( new RWBranchDesc( brname, 0.99, sigpls, sigmin ) ); 
       
   //
   // Create "copy" Event tree
   //
   TTree* cpevtree = new TTree( "gtree", "copy of GENIE events to reweight" );
   cpevtree->SetAutoSave( 200000000 );
   genie::NtpMCEventRecord* cpmcrec = 0;
   TBranch* cpevbr = cpevtree->Branch( "gmcrec", "genie::NtpMCEventRecord", &cpmcrec, 32000, 1 ); 
   assert(cpevbr);
   
   // "Tie" these trees together !
   //
   cpevtree->AddFriend( rwtree );

   // input GENIE sample
   //
   std::string isample = "/scratch-shared/yarba_j/tmp-genie/nu-Hydrocarbon/100K/gntp.1.ghep.root";
   TFile* file = new TFile( isample.c_str(), "READ" );
      
   if ( !file ) return 1;
   
   // Fetch the Evt tree
   //
   TTree* evtree = dynamic_cast<TTree*>( file->Get("gtree")  );
   
   // Connect Evt record (branch)
   //
   genie::NtpMCEventRecord* mcrec = 0;
   evtree->SetBranchAddress( "gmcrec", &mcrec );
   
   int nevt_total = evtree->GetEntries();   
   
   int nevt_to_reweight = std::min( nevt_total, 100 );
      
   double twk = 0.;
   double wt = 1.;
   int iev = 0;
   int nrw = 0;
      
//   for ( int iev=0; iev<nevt_to_reweight; ++iev )
   while ( iev < nevt_total && nrw < nevt_to_reweight )
   {
   
       evtree->GetEntry(iev);
       genie::EventRecord& evt = *(mcrec->event);
       iev++;

       //
       // Select events to be re-weighted
       //
       // Specifically, check if it's QEL && WeakCC process
       // because that's what we want to reweight (MaCCQE).
       // Also skip charm events (although if those are quite rare)
       //
       genie::Interaction* interaction = evt.Summary();    
       const genie::ProcessInfo& prinfo = interaction->ProcInfo();   
       const genie::XclsTag&     xclsv  = interaction->ExclTag();
       bool accept = ( prinfo.IsQuasiElastic() && prinfo.IsWeakCC() && !xclsv.IsCharmEvent() );
       if ( !accept ) continue ;
          
       rwrec = new RWRecord();
       rwrec->SetOriginalEvtNumber( iev );

       twk = -0.5; // in the units of MaCCQE SIGMA !!! (that's who the weigh calculator "understands" it)
       syst.Set( genie::rew::kXSecTwkDial_MaCCQE, twk );
       rw.Reconfigure();
       wt = rw.CalcWeight(evt);
       rwrec->Insert( twk, wt );
       
       twk = 0.5;
       syst.Set( genie::rew::kXSecTwkDial_MaCCQE, twk );
       rw.Reconfigure();
       wt = rw.CalcWeight(evt);
       rwrec->Insert( twk, wt );
       
       rwtree->Fill();
       
       // now copy over re-weighted event(s)
       //
       cpmcrec = new genie::NtpMCEventRecord();
       cpmcrec->Fill( iev, &evt ); // for now, use iev counter to keep the original numbering
       
       cpevtree->Fill();
       
       delete rwrec;
       rwrec = 0;
       delete cpmcrec;
       cpmcrec = 0;
       
       nrw++;
       
       // Clear mc evt record before the next one
       //
       mcrec->Clear();
   
   }
  
   ofile->cd();   
   rwtree->Write();
   cpevtree->Write();
   
   delete rwtree;
   rwtree=0;
   delete cpevtree;
   cpevtree=0;

   ofile->Close(); 
   file->Close();

/*     
   // RE_TEST NOW !!!
   //   
   TFile* tfile = new TFile( "test_rw.root", "READ" );
   //
   // Fetch the RW tree
   //
   TTree* rwtree_test = dynamic_cast<TTree*>( tfile->Get("reweighting") );
   
   TList* hdr = rwtree_test->GetUserInfo();
   int nhdr = hdr->GetEntries();
   for ( int i=0; i<nhdr; ++i )
   {
      RWBranchDesc* brdesc = dynamic_cast<RWBranchDesc*>( hdr->At(i) );
      std::cout << " branch name: " << brdesc->GetParameterName() << std::endl;
      std::cout << " parameter: " << brdesc->GetParameterMean() << " " 
                << brdesc->GetParameterSigmaPlus() << " " << brdesc->GetParameterSigmaMinus() << std::endl; 
   }
   
   nrw = 0;
   nrw = rwtree_test->GetEntries();
   std::cout << " num of entries of re-test: " << nrw << std::endl;   
   RWRecord* rwrec_test = 0;
   rwtree_test->SetBranchAddress( brname.c_str(), &rwrec_test );   
   for ( int i=0; i<nrw; ++i )
   {
       rwtree_test->GetEntry(i);
       // genie::EventRecord& evt = *(mcrec->event);
       int nres = rwrec_test->GetNumOfRWResults();
       for ( int ir=0; ir<nres; ++ir )
       {
          double twk_test = rwrec_test->GetTweak( ir );
	  double wt_test  = rwrec_test->GetWeight( ir );
	  std::cout << " twk = " << twk_test << " wt = " << wt_test << std::endl;
       }
   }
*/
       
   return 0;

}

