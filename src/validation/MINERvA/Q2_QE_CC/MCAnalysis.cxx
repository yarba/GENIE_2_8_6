#include <cstdlib>
#include <cassert>
#include <map>
#include <vector>

#include <iostream>
#include <sstream>

#include <TFile.h>
#include <TTree.h>

#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCEventRecord.h"
#include "Ntuple/NtpMCRecHeader.h"
#include "Ntuple/NtpMCTreeHeader.h"
#include "Messenger/Messenger.h"
#include "PDG/PDGCodes.h"
#include "Conventions/Units.h"

#include "Interaction/Interaction.h"
#include "Interaction/Target.h"

#include "MCAnalysis.h"
#include "Q2QEAnalyzer.h"
#include "ExpData.h"

#include "validation/MINERvA/ReWeightIO/RWRecord.h"

using namespace genie;

MCAnalysis::MCAnalysis() 
   : fEvtTree(0), fMCRec(0), fRWTree(0), fRWRec(0) 
{ 
}

MCAnalysis::~MCAnalysis()
{

   for ( size_t i=0; i<fAnalyzers.size(); ++i )
   {
      delete fAnalyzers[i];
   }
   fAnalyzers.clear();
   
}

void MCAnalysis::AddMCAnalyzer( const ExpDataSet* dset )
{

   assert(dset);
   
   const std::string& oname = dset->GetObservable();
   
   if ( oname == "Q2QE" )
   {
      fAnalyzers.push_back( new Q2QEAnalyzer() );
   }
   else
   {
      LOG("gvldtest", pWARN) 
        << " Analyzer for observable " << oname << " does NOT exist; SKIP " ;
      return;
   }
   
   fAnalyzers.back()->Init( dset->GetInteraction(), 
                            oname, (
			    dset->GetDataHisto()).GetCVHistoWithStatError() ); 
      
   return;

}

const BaseAnalyzer* MCAnalysis::GetMCAnalyzer( int i ) const
{

   int NAnz = fAnalyzers.size();
   
   if ( NAnz == 0 || i >= NAnz )
   {
      return NULL;
   }
   
   return fAnalyzers[i];

}

const BaseAnalyzer* MCAnalysis::GetMCAnalyzer( const ExpDataSet* dset ) const
{
   
   for ( unsigned i=0; i<fAnalyzers.size(); ++i )
   {
      if ( fAnalyzers[i]->GetObservable() == dset->GetObservable() && 
           fAnalyzers[i]->GetInteraction() == dset->GetInteraction() ) return fAnalyzers[i];
   }
   
   
   return NULL;

}

void MCAnalysis::Analyze( const std::string& sample )
{

   // for more info of GENIE event/analysis see GENIE manual, chapter 6 (p.101) 
   // for examples of analysis app/skeleton see GENIE manual, section 6.4 (p.106)
      
   LOG("gvldtest", pNOTICE) 
     << "Analyzing input file " << sample ;
      
   // open up GENIE sample (output of gevgen)
   //
   TFile* fin = new TFile( sample.c_str(), "READ" );
   if ( !fin )
   {
      LOG("gvldtest", pERROR) 
         << "Invalid Genie sample " << sample;
      return;
   }

   // connect to event tree 
   //
   fEvtTree = dynamic_cast <TTree *>( fin->Get("gtree")  );

   if (!fEvtTree) 
   {
      LOG("gvldtest", pERROR) 
         << "Null input GHEP event tree found in file: " << sample;
      return;
   }
      
   //  set the event record pointer at the start of the tree
   //
   fEvtTree->SetBranchAddress("gmcrec", &fMCRec);   

   // Check if the address of the event record is valid
   //
   if (!fMCRec) 
   {
      LOG("gvldtest", pERROR) << "Null MC record";
      return;
   }
   
   // Connect the reweighting tree (if any)
   //
   fRWTree = dynamic_cast<TTree*>( fin->Get("reweighting") );
   if ( fRWTree )
   {
      fEvtTree->AddFriend( fRWTree );
      fRWTree->SetBranchAddress( "MaCCQE", &fRWRec );
   }
   
   // find out how many generated events are in the tree 
   //
   Long64_t NEvents = fEvtTree->GetEntries();
   if ( NEvents <= 0 ) 
   {
      LOG("gvldtest", pERROR) << "Number of events = 0";
      return;
   }
   
   LOG("gvldtest", pNOTICE) 
     << "*** Analyzing: " << NEvents << " events found in file " << sample;
     
   double XSec = 0.;
   
   // Loop over events in the tree
   //
   for(Long64_t iev = 0; iev < NEvents; ++iev) 
   {
      
      // fetch event records one by one 
      //
      fEvtTree->GetEntry(iev);
      
      for ( unsigned int i=0; i<fAnalyzers.size(); ++i )
      {
         fAnalyzers[i]->Analyze( fMCRec, fRWRec );
      }
      
      //
      // This is an example how you can common calc.,
      // for example, calc. total xsec from the diff. xsec's
      // NOTE: the evernt.XSec() is a diff.xsec for an event of THIS kinematics
      //
      EventRecord&   event      = *(fMCRec->event); 
      double xsec = event.XSec();
      xsec *= (1E+38/units::cm2); // for more info on unites - see GENIE manual, section 6.7 (p.122)
      XSec += xsec;
            
      fMCRec->Clear();
      if ( fRWRec ) fRWRec->Reset();
            
   } // end loop over NEvents

   XSec /= NEvents;

   fin->Close();
   delete fin;
   
   fEvtTree = 0;
   fMCRec = 0;
   fRWTree  = 0;
   fRWRec = 0;

   return;

}
