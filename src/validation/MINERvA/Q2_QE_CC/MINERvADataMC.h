#ifndef Val_MINERvA_MINERvADataMC_H
#define Val_MINERvA_MINERvADataMC_H

#include "ResultsDataMC.h"

namespace PlotUtils {
class MUPlotter;
class MUH1D;
}

class MINERvADataMC : public ResultsDataMC
{

   public:
   
     // ctor & dtor
     MINERvADataMC();
     virtual ~MINERvADataMC();
     
   protected:
     
      virtual void DoIt( const ExpDataSet*, const BaseAnalyzer* );   

   private:
   
      // private member functions
      // 
      // this one below is mainly for debugging purposes 
      // but it'll show how to extract info on matrices, etc.
      //
      void ServicePrint( const PlotUtils::MUH1D* ) const;
      
      // data members
      PlotUtils::MUPlotter* fPlotter;

};


#endif
