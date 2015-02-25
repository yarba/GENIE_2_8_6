
#include <TRootIOCtor.h>

#include "RWBranchDesc.h"

#include <cassert>

ClassImp(RWBranchDesc)

RWBranchDesc::RWBranchDesc( const std::string& name, 
                            const double mean, const double sigpls, const double sigmin )
   : TObject()
{

   SetParameter( name, mean, sigpls, sigmin );

}

RWBranchDesc::RWBranchDesc( const RWBranchDesc& brdesc )
   : TObject()
{

   fParameterName  = brdesc.fParameterName;
   fParameterMean  = brdesc.fParameterMean;
   fParameterSigmaPlus = brdesc.fParameterSigmaPlus;
   fParameterSigmaMinus = brdesc.fParameterSigmaMinus;

}

RWBranchDesc::RWBranchDesc( TRootIOCtor* )
   : TObject(), fParameterName(), fParameterMean(0.), fParameterSigmaPlus(0.), fParameterSigmaMinus(0.)
{
}

void RWBranchDesc::SetParameter( const std::string& name, 
                                 const double mean, const double sigpls, const double sigmin )
{

   fParameterName = name;
   fParameterMean = mean;
   fParameterSigmaPlus = sigpls;
   fParameterSigmaMinus = sigmin;

   return;

}
