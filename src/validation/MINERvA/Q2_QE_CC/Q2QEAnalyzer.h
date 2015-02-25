#ifndef Val_MINERvA_Q2QEAnalyzer_H
#define Val_MINERvA_Q2QEAnalyzer_H

#include "BaseAnalyzer.h"

class Q2QEAnalyzer : public BaseAnalyzer
{

   public:
      
      // ctor & dtor
      Q2QEAnalyzer() : BaseAnalyzer() { fObservable="Q2QE"; /* fXSec=0.; */ };
      virtual ~Q2QEAnalyzer() {}; // nothing to do, everything will be done on Base
            
      virtual void Reset();
      virtual void EndOfRun();
      
   protected:
   
      virtual void DoIt( genie::NtpMCEventRecord*, RWRecord* rwrec=0 );

};

#endif
