#include <sstream>
#include <cassert>
#include <algorithm>

#include "BaseRWProducer.h"

#include <TSystem.h>

#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCFormat.h"
#include "Ntuple/NtpMCTreeHeader.h"
#include "Ntuple/NtpMCEventRecord.h"
#include "Messenger/Messenger.h"
#include "Numerical/RandomGen.h"
#include "PDG/PDGCodes.h"
#include "PDG/PDGCodeList.h"

#include "ReWeight/GSystUncertainty.h"
// --> ??? #include "ReWeight/GReWeightUtils.h"

// I/O for re-weighting info 
//
#include "RWBranchDesc.h"
#include "RWRecord.h"

BaseRWProducer::BaseRWProducer()
   : fRWTree(0), fExistingRWTree(false), fInputFile(0), fOutputFile(0)
{

   fNu2ReWeight.push_back( genie::kPdgNuE       );
   fNu2ReWeight.push_back( genie::kPdgAntiNuE   );
   fNu2ReWeight.push_back( genie::kPdgNuMu      );
   fNu2ReWeight.push_back( genie::kPdgAntiNuMu  );
   fNu2ReWeight.push_back( genie::kPdgNuTau     );
   fNu2ReWeight.push_back( genie::kPdgAntiNuTau );
      
   fOutputMode = "UPDATE";

}

BaseRWProducer::BaseRWProducer( const std::string& pname, const std::vector<double>& twkdials, const std::string& outmode )
   : fRWTree(0), fExistingRWTree(false), fInputFile(0), fOutputFile(0)
{

   fNu2ReWeight.push_back( genie::kPdgNuE       );
   fNu2ReWeight.push_back( genie::kPdgAntiNuE   );
   fNu2ReWeight.push_back( genie::kPdgNuMu      );
   fNu2ReWeight.push_back( genie::kPdgAntiNuMu  );
   fNu2ReWeight.push_back( genie::kPdgNuTau     );
   fNu2ReWeight.push_back( genie::kPdgAntiNuTau );

   fParam2Tweak = genie::rew::GSyst::FromString( pname );
   fTwkDials = twkdials; 
   fOutputMode = outmode;
   std::string tmp = outmode; 
   std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);  
   if ( tmp == "cherry-pick" || tmp == "new" || tmp == "recreate" ) fOutputMode = "RECREATE"; 

}

BaseRWProducer::~BaseRWProducer()
{

  fTwkDials.clear();
  Reset();
  
}

void BaseRWProducer::UpdateNu2ReWeightList( const std::vector<int>& nulist )
{

   if ( nulist.empty() ) return;
   
   fNu2ReWeight.clear();
   //
   // unfortunately, can't use "=" operator between vector<int> & PDGCodeList
   //
   for ( unsigned int i=0; i<nulist.size(); ++i )
   {
      fNu2ReWeight.push_back( nulist[i] );
   }

   return;

}

void BaseRWProducer::Reset()
{
  
   if ( !fExistingRWTree )
   {
      if ( fRWTree ) delete fRWTree;
   }
   fRWTree = 0 ;
   fExistingRWTree = false;

   if ( fInputFile )
   {
      if ( fInputFile->IsOpen() ) fInputFile->Close();
   }
  
   if ( fOutputFile )
   {
      if ( fOutputFile->IsOpen() ) fOutputFile->Close(); 
   }

   return;

}

void BaseRWProducer::Init( const std::string& sample )
{

   assert(!(genie::rew::GSyst::AsString(fParam2Tweak)).empty());

   DoInit(); // purely vertual methood !!!

   if ( fOutputMode == "UPDATE" ) // write into existing sample
   {
      fInputFile = new TFile( sample.c_str(), fOutputMode.c_str() );
      fRWTree = dynamic_cast<TTree*>( fInputFile->Get("reweighting") );
      // fOutputFile = fInputFile; // this doesn't work...
   }
   else // write to new sample
   {
      fInputFile = new TFile( sample.c_str(), "READ" );
      std::string osample = "test_rw.root";      // <---- FIXME !!!
      fOutputFile = new TFile( sample.c_str(), fOutputMode.c_str() );  // in principle, one can also do "UPDATE" instead of "RECREATE"
      fRWTree = dynamic_cast<TTree*>( fOutputFile->Get("reweighting") );
   }

   // FIXME !!! Check here examples of TTree override and whether or not it should be new/delete !!!
   
   //
   // If no RW tree, create it 
   //
   if ( !fRWTree )
   {
      fRWTree = new TTree( "reweighting", "GENIE weights tree" );
      TTree::SetBranchStyle(1);
      fRWTree->SetAutoSave( 200000000 );  // autosave when 0.2 Gbyte written 
                                          // - it's the same for "gtree" but I need to double check 
				          // how to *get* autosave from "gtree" 
   }
   else
   {
      fExistingRWTree = true;
   }
   
   // Add meta-data (UserInfo) to the RW tree
   //
   // Fisrt of all, check if metadata for this syst.param already exist
   //
   if ( fRWTree->GetUserInfo()->GetSize() > 0 )
   {
      int nmd = fRWTree->GetUserInfo()->GetSize();
      for ( int imd=0; imd<nmd; ++imd )
      {
         RWBranchDesc* bd = dynamic_cast<RWBranchDesc*>( fRWTree->GetUserInfo()->At(imd) );
	 if ( bd->GetParameterName() == genie::rew::GSyst::AsString(fParam2Tweak) )
	 {
	    std::cout << " Metadata already exist for syst.parameter " 
	              << genie::rew::GSyst::AsString(fParam2Tweak) << std::endl;
	    return; // Do NOT override for now, although in principle, one should probably remove the existing one and replace
	 }
      }   
   }
      
   // MaCCQE=0.99 has been extracted under gdb from GReWeightNuXSecCCQE class (member data fMaDef);
   // in principle, it depends on the physics model - in the case of GReWeightNuXSecCCQE the model 
   // is based on the "LwlynSmithQELCCPXSec" algorithm (see GReWeightNuXSecCCQE::Init() method)
   //
   // Sigma's (+/-) can be extracted from GSystUncertainty
   //
   genie::rew::GSystUncertainty* syser = genie::rew::GSystUncertainty::Instance();
   double sigpls = syser->OneSigmaErr( fParam2Tweak,  1 );
   double sigmin = syser->OneSigmaErr( fParam2Tweak, -1 );
   //
   fRWTree->GetUserInfo()->Add( new RWBranchDesc( (genie::rew::GSyst::AsString(fParam2Tweak)).c_str(), 
                                                  0.99, sigpls, sigmin ) ); 

   return;

}

void BaseRWProducer::ReWeight( const std::string& sample )
{

   Reset();
   Init( sample );

   // Create a branch to correspond to a specific parameter/variable to vary
   //
   RWRecord* rwrec = 0;
   std::string param_name = genie::rew::GSyst::AsString( fParam2Tweak );  
   TBranch* rwbr = 0;
   rwbr = fRWTree->GetBranch( param_name.c_str() );
   if ( !rwbr )
   {
      rwbr = fRWTree->Branch( param_name.c_str(), "RWRecord", &rwrec, 32000, 1 ); // FIXME !!! also check more "sophisticated" options  
      assert(rwbr); 
      rwbr->SetAutoDelete(kFALSE);  
   }
   else
   {
      std::cout << " WARNING !!! You are going to override an existing branch ! " << std::endl;
      fRWTree->SetBranchAddress( param_name.c_str(), &rwrec );
   }
   
   // Fetch the Evt tree
   //
   TTree* evtree = dynamic_cast<TTree*>( fInputFile->Get("gtree")  );
   
   // Copy event tree (for cherry-pick mode)
   //
   TTree* cpevtree = 0;
   genie::NtpMCEventRecord* cpmcrec = 0;
   if ( fOutputMode != "UPDATE" )
   {
      cpevtree = new TTree( "gtree", "copy of GENIE events to reweight" );
      assert( cpevtree );
      cpevtree->SetAutoSave( 200000000 );
      TBranch* cpevbr = cpevtree->Branch( "gmcrec", "genie::NtpMCEventRecord", &cpmcrec, 32000, 1 ); 
      assert( cpevbr );
      cpevtree->AddFriend( fRWTree );
   }

   // Connect Evt record (branch)
   //
   genie::NtpMCEventRecord* mcrec = 0;
   evtree->SetBranchAddress( "gmcrec", &mcrec );

   int nevt_total = evtree->GetEntries();   
   
   for ( int iev=0; iev<nevt_total; ++iev )
   {
      
      evtree->GetEntry(iev);
      
      rwrec = new RWRecord();
      rwrec->SetOriginalEvtNumber( iev );
      
      bool status = DoIt( mcrec, rwrec ); // purely virtual method !!!
                                          // checks if the event is good to re-weight; if not, it'll NOT populate rwrec
      
      fRWTree->Fill();
      delete rwrec;
      rwrec = 0;
      
      if ( ! status ) 
      {
	 mcrec->Clear();
         continue;
      }
            
      // create a copy event if needs be
      //
      if ( fOutputMode != "UPDATE" )
      {
         cpmcrec = new genie::NtpMCEventRecord();
         cpmcrec->Fill( iev, mcrec->event ); // for now, use iev counter to keep the original numbering       
         cpevtree->Fill();
         delete cpmcrec;
         cpmcrec = 0;
      }
       
      
      mcrec->Clear();
            
      // FIXME !!!
      // Try to figure out if the tree of copy-events can be expanded 
      // by inserting additional events... 
      // ... if that needs to be ???...
            
   }   
   
   // check for consistency - # of branches should be equal to the # of branch descriptions
   //
   if ( fRWTree->GetNbranches() != fRWTree->GetUserInfo()->GetSize() ) // NOTE: GetSize() == GetEntries()
   {
      std::cout << "WARNING: NUMBER OF BRANCHES IS DIFFERENT FROM THE NUMBER OF BRANCH METADATA" << std::endl;
   }
   
   if ( fOutputMode == "UPDATE") 
   {
      fInputFile->cd();
   }
   else
   {
      fOutputFile->cd();
   }
   fRWTree->Write("",TObject::kOverwrite);
   if ( cpevtree ) 
   {
      cpevtree->Write("",TObject::kOverwrite);
      delete cpevtree;
      cpevtree=0;
   }
   
   Reset();

   return;

}
