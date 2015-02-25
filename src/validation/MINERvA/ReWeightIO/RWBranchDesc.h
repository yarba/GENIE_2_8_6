#ifndef RWBRANCHDESC_H
#define RWBRANCHDESC_H

#include <string>
#include <TObject.h>

class TRootIOCtor;

class RWBranchDesc : public TObject
{

   public:
   
      RWBranchDesc() : TObject(), fParameterName(), fParameterMean(0.), fParameterSigmaPlus(0.), fParameterSigmaMinus(0.) {} 
      RWBranchDesc( const std::string&, const double, const double, const double );
      RWBranchDesc( const RWBranchDesc& );
      RWBranchDesc( TRootIOCtor* );
      
      ~RWBranchDesc() {}
      
      const std::string& GetParameterName()       const { return fParameterName; }
      double             GetParameterMean()       const { return fParameterMean; }
      double             GetParameterSigmaPlus()  const { return fParameterSigmaPlus; }
      double             GetParameterSigmaMinus() const { return fParameterSigmaMinus; }
      
      void               SetParameter( const std::string&, const double, const double, const double );
         
   private:
   
      std::string fParameterName;
      double      fParameterMean;
      double      fParameterSigmaPlus;
      double      fParameterSigmaMinus;


ClassDef(RWBranchDesc,2)

};

#endif
