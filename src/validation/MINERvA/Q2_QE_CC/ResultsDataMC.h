#ifndef Val_MINERvA_ResultsDataMC_H
#define Val_MINERvA_ResultsDataMC_H

class ExpData;
class ExpDataSet;
class MCAnalysis;
class BaseAnalyzer;

class ResultsDataMC
{

   public:
      
      // ctor & dtor
      ResultsDataMC() {};
      virtual ~ResultsDataMC() {};
      
      virtual void FinalizeResults( const ExpData*, const MCAnalysis* );

   protected:
   
      virtual void DoIt( const ExpDataSet*, const BaseAnalyzer* ) = 0;

};

#endif
