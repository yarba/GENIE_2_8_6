#ifndef Val_MINERvA_BaseAnalyzer_H
#define Val_MINERvA_BaseAnalyzer_H

#include <string>
#include "IntType.h"

// #include "TH1.h"
//
#include "PlotUtils/MUH1D.h"

class TTree;
class TArrayF;
// class TH1D;

namespace genie {
class NtpMCEventRecord;
}

class BaseAnalyzer
{

   public:
   
      // ctor & dtor
      //
      BaseAnalyzer(); 
      virtual ~BaseAnalyzer();

      virtual void Init( const IntType&, const std::string&, const TH1D& );
      void Analyze( genie::NtpMCEventRecord*, TArrayF* weights=0 );
      
      bool IsUnphysical() const;
      
      virtual void     Reset() = 0;
      virtual void     EndOfRun() = 0; 
      
      const std::string&      GetObservable()  const { return fObservable; }
      const IntType&          GetInteraction() const { return fInteractionType; }

      PlotUtils::MUH1D        GetSimHistoFluxNormalized() const; 
      PlotUtils::MUH1D        GetSimHistoAsShape()        const; 
        
   protected:

      // data members
      //
      virtual void DoIt( genie::NtpMCEventRecord*, TArrayF* weights=0 ) = 0;
      
      void SetInteraction( const IntType& type ) { fInteractionType=type; return; }
      void SetInteraction( const int prjpdg, const int nucpdg ) { fInteractionType.SetType( prjpdg, nucpdg ); return; }
      
      // data members
      //
      IntType                    fInteractionType;
      std::string                fObservable;
      double                     fXSec;      
      PlotUtils::MUH1D*          fSimHisto;
      bool                       fEndOfRunApplied;

};



#endif