#ifndef Val_MINERvA_BaseAnalyzer_H
#define Val_MINERvA_BaseAnalyzer_H

#include <string>
#include "IntType.h"

// #include "TH1.h"
//
#include "PlotUtils/MUH1D.h"

class TTree;
// class TH1D;

namespace genie {
class NtpMCEventRecord;
}

class Analyzer
{

   public:
      
      Analyzer();
      ~Analyzer();
      
      Init( const IntType&, const TH1D* );
      void SetObservable( const std::string& )

      void Analyze( genie::NtpMCEventRecord*, std::vector<double>* weights=0 );

      const std::string&      GetObservable() const { return fObservable; }
      PlotUtils::MUH1D*       GetSimHisto()   const { return fSimHisto; }

   private:
      
      void SetInteraction( const IntType& type ) { fInteractionType=type; return; }
      void SetInteraction( const int prjpdg, const int nucpdg ) { fInteractionType.SetType( prjpdg, nucpdg ); return; }
   
      IntType                    fInteractionType;
      std::string                fObservable;      
      PlotUtils::MUH1D*          fSimHisto;
      bool                       fEndOfRunApplied;


};


#endif
