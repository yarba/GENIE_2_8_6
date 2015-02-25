#include <string>
#include <sstream>
#include <cassert>

#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
// #include <TArrayF.h>

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

// Modules to calc weights/uncertainties
// by varying specific variables
//
// This one right below is for MaCCQE
//
#include "ReWeight/GReWeightNuXSecCCQE.h"
//
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

// I/O for re-weighting info 
//
#include "RWBranchDesc.h"
#include "RWRecord.h"
  
int main( int argc, char ** argv )
{
      
   // First and foremost, define what parameter we want to vary/tweak
   //
   // FIXME !!!
   // In principle, it should be run-time configurable
   //
//   genie::rew::EGSyst param_to_tweak = genie::rew::kXSecTwkDial_MaCCQE ;
   genie::rew::GSyst_t param_to_tweak = genie::rew::kXSecTwkDial_MaCCQE ;

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
   // syst.Init( genie::rew::kXSecTwkDial_MaCCQE );
   syst.Init( param_to_tweak );
   
   // By default GReWeightNuXSecCCQE is in `NormAndMaShape' mode 
   // where Ma affects the shape of dsigma/dQ2 and a different param affects the normalization
   // If the input is MaCCQE, switch the weight calculator to `Ma' mode
   //
   genie::rew::GReWeightNuXSecCCQE* rwccqe = dynamic_cast<genie::rew::GReWeightNuXSecCCQE*>( rw.WghtCalc("xsec_ccqe") );  
   rwccqe->SetMode( genie::rew::GReWeightNuXSecCCQE::kModeMa );

   //
   // Create a tree for RW records (new and "up in the air")
   //
   TTree* rwtree = new TTree( "reweighting", "GENIE weights tree" );
   TTree::SetBranchStyle(1);
   rwtree->SetAutoSave( 200000000 );  // autosave when 0.2 Gbyte written 
                                      // - it's the same for "gtree" but I need to double check 
				      // how to *get* autosave from "gtree" 
   
   // Now create a branch to correspond to a specific parameter/variable to vary
   //
   RWRecord* rwrec = 0;
   std::string param_name = genie::rew::GSyst::AsString( param_to_tweak ); 
   TBranch* rwbr = rwtree->Branch( param_name.c_str(), "RWRecord", &rwrec, 32000, 1 ); // FIXME !!! also check more "sophisticated" options  
   assert(rwbr); 
   rwbr->SetAutoDelete(kFALSE);  

   // Add meta-data (UserInfo) to the RW tree
   //
   // MaCCQE=0.99 has been extracted under gdb from GReWeightNuXSecCCQE class (member data fMaDef);
   // in principle, it depends on the physics model - in the case of GReWeightNuXSecCCQE the model 
   // is based on the "LwlynSmithQELCCPXSec" algorithm (see GReWeightNuXSecCCQE::Init() method)
   //
   // Sigma's (+/-) can be extracted from GSystUncertainty
   //
   genie::rew::GSystUncertainty* syser = genie::rew::GSystUncertainty::Instance();
//   double sigpls = syser->OneSigmaErr( genie::rew::kXSecTwkDial_MaCCQE,  1 );
//   double sigmin = syser->OneSigmaErr( genie::rew::kXSecTwkDial_MaCCQE, -1 );
   double sigpls = syser->OneSigmaErr( param_to_tweak,  1 );
   double sigmin = syser->OneSigmaErr( param_to_tweak, -1 );
   //
   rwtree->GetUserInfo()->Add( new RWBranchDesc( param_name, 0.99, sigpls, sigmin ) ); 
   
   // Now open input GENIE sample
   //
   std::string isample = "/scratch-shared/yarba_j/tmp-genie/nu-Hydrocarbon/rw-testing/100K/gntp.1.ghep.root";
   TFile* file = new TFile( isample.c_str(), "UPDATE" );
      
   // if invalid input file, bail out
   //
   if ( !file ) return 1;
   
   // OK, input file is valid
   //
   // Fetch the Evt tree
   //
   TTree* evtree = dynamic_cast<TTree*>( file->Get("gtree")  );
   
   // Connect Evt record (branch)
   //
   genie::NtpMCEventRecord* mcrec = 0;
   evtree->SetBranchAddress( "gmcrec", &mcrec );
   
   // "Tie" together these trees, Evt & RW !
   //
   evtree->AddFriend( rwtree );

   
   // now loop over events and see what needs to be re-weighted
   //
   int nevt_total = evtree->GetEntries();   
      
   double twk = 0.;
   double wt = 1.;
      
   for ( int iev=0; iev<nevt_total; ++iev )
   {
   
       evtree->GetEntry(iev);
       genie::EventRecord& evt = *(mcrec->event);
       
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
       if ( !accept ) 
       {
          rwrec = new RWRecord();
          rwrec->SetOriginalEvtNumber(iev);
          rwtree->Fill();
          delete rwrec;
          rwrec=0;
          continue ;
       }
          
       rwrec = new RWRecord();
       
       rwrec->SetOriginalEvtNumber( iev );

       twk = -0.5; // in the units of MaCCQE SIGMA !!! (that's who the weigh calculator "understands" it)
//       syst.Set( genie::rew::kXSecTwkDial_MaCCQE, twk );
       syst.Set( param_to_tweak, twk );
       rw.Reconfigure();
       wt = rw.CalcWeight(evt);
       rwrec->Insert( twk, wt );
       
       twk = 0.5;
//       syst.Set( genie::rew::kXSecTwkDial_MaCCQE, twk );
       syst.Set( param_to_tweak, twk );
       rw.Reconfigure();
       wt = rw.CalcWeight(evt);
       rwrec->Insert( twk, wt );
       
       rwtree->Fill();
              
       // Clear mc evt record before the next one
       //
       mcrec->Clear();
       if ( rwrec ) delete rwrec;
       rwrec = 0;
         
   }
  
   file->cd();
   rwtree->Write("",TObject::kOverwrite);
   
   delete rwtree;
   rwtree=0;

   file->Close();
     
/*
   // RE_TEST NOW !!!
   //   
   TFile* tfile = new TFile( "/scratch-shared/yarba_j/tmp-genie/nu-Hydrocarbon/rw-testing/100K/gntp.1.ghep.root", "READ" );

   //
   // Fetch the RW tree
   //
   TTree* rwtree_test = dynamic_cast<TTree*>( tfile->Get("reweighting") );
   
   TList* hdr = rwtree_test->GetUserInfo();
   assert(hdr);
      
   int nentries = rwtree_test->GetEntries();
   int nrw = 0;
   std::cout << " num of entries of re-test: " << nentries << std::endl;   
   RWRecord* rwrec_test = 0;
   rwtree_test->SetBranchAddress( param_name.c_str(), &rwrec_test );   
   for ( int i=0; i<nentries; ++i )
   {
       rwtree_test->GetEntry(i);
       // genie::EventRecord& evt = *(mcrec->event);
       int nres = rwrec_test->GetNumOfRWResults();
       if ( nres <= 0 ) continue;
       for ( int ir=0; ir<nres; ++ir )
       {
          double twk_test = rwrec_test->GetTweak( ir );
	  double wt_test  = rwrec_test->GetWeight( ir );
	  std::cout << " twk = " << twk_test << " wt = " << wt_test << std::endl;
       }
       nrw++;
   }

   std::cout << " num of re-weighted results of re-test: " << nrw << std::endl;   

   // now print meta-data
   //
   int nhdr = hdr->GetEntries();
   for ( int i=0; i<nhdr; ++i )
   {
      RWBranchDesc* brdesc = dynamic_cast<RWBranchDesc*>( hdr->At(i) );
      std::cout << " branch name: " << brdesc->GetParameterName() << std::endl;
      std::cout << " parameter: " << brdesc->GetParameterMean() << " " 
                << brdesc->GetParameterSigmaPlus() << " " << brdesc->GetParameterSigmaMinus() << std::endl; 
   }
*/

   return 0;

}

