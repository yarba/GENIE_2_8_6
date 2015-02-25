#ifndef RWPRODUCERCCQE_H
#define RWPRODUCERCCQE_H

#include "BaseRWProducer.h"

class RWProducerCCQE : public BaseRWProducer {

   public:      
   
      // ctor & dtor
      RWProducerCCQE() : BaseRWProducer() {}
      RWProducerCCQE( const std::string pname, const std::vector<double>& twk, const std::string& outmode ) : BaseRWProducer(pname,twk,outmode) {}
      virtual ~RWProducerCCQE() {}
   
   protected:
   
      virtual void DoInit();
      virtual bool DoIt( genie::NtpMCEventRecord*, RWRecord* );

};

#endif
