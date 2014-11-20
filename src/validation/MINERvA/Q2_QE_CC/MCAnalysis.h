#ifndef Val_MINERvA_MCAnalysis_H
#define Val_MINERvA_MCAnalysis_H

#include <string>
#include "IntType.h"
#include "BaseAnalyzer.h"

class TTree;
class TH1D;

namespace genie {
// fwd declaration (namespace genie)
class NtpMCEventRecord;
}

class ExpDataSet;

class MCAnalysis
{

   public:
   
      // ctor & dtor
      //
      MCAnalysis(); 
      ~MCAnalysis();

      // public member fuctions
      //
      void AddMCAnalyzer( const ExpDataSet* );
      void Reset()    { for ( unsigned int i=0; i<fAnalyzers.size(); ++i )fAnalyzers[i]->Reset(); return; }
      void EndOfRun() { for ( unsigned int i=0; i<fAnalyzers.size(); ++i )fAnalyzers[i]->EndOfRun(); return; } // FIXME: in principle,
                                                                                                               // it's EndOfRun( double scale )...
      void Analyze( const std::string&, const std::string& rwsample="" );
      
      const int           GetNAnalyzers() const { return fAnalyzers.size(); }
      const BaseAnalyzer* GetMCAnalyzer( int i ) const;
      const BaseAnalyzer* GetMCAnalyzer( const ExpDataSet* ) const;

   private:

      // data members
      //
      TTree*                     fEvtTree;
      genie::NtpMCEventRecord*   fMCRec;              
      std::vector<BaseAnalyzer*> fAnalyzers;

};



#endif
