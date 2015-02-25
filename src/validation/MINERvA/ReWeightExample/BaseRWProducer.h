#ifndef BASERWPRODUCER_H
#define BASERWPRODUCER_H

#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>

#include "PDG/PDGCodeList.h"

#include "ReWeight/GReWeightI.h"
#include "ReWeight/GSystSet.h"
#include "ReWeight/GSyst.h"
#include "ReWeight/GReWeight.h"

namespace genie {
class NtpMCEventRecord;
}
class RWRecord;

class BaseRWProducer {

   public:
   
      // ctor & dtor
      //
      BaseRWProducer();
      BaseRWProducer( const std::string&, const std::vector<double>&, const std::string& );
      virtual ~BaseRWProducer();

      void UpdateNu2ReWeightList( const std::vector<int>& );
      void ReWeight( const std::string& );
      void Reset();      

   protected:
   
      void         Init( const std::string& );
      virtual void DoInit() = 0;
      virtual bool DoIt( genie::NtpMCEventRecord*, RWRecord* ) = 0;
   
      genie::rew::GReWeight fGReWeight;  // this is an object, not a pointer ! 
      genie::PDGCodeList    fNu2ReWeight;   
      genie::rew::GSyst_t   fParam2Tweak;      
      std::vector<double>   fTwkDials;
      TTree*                fRWTree;
      bool                  fExistingRWTree;
      TFile*                fInputFile;
      TFile*                fOutputFile;
      std::string           fOutputMode; // cherry-pick/skim or update

};

#endif
