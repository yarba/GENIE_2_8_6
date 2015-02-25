
#include <TRootIOCtor.h>

#include "RWRecord.h"

#include <cassert>

ClassImp(RWRecord)

RWRecord::RWRecord()
   : TObject(),
     fOrigEvtNum(-1)
{

   fRWResults.clear();

}

RWRecord::RWRecord( const RWRecord& rwrec )
   : TObject()
{

   this->Copy(rwrec);

}

RWRecord::RWRecord( TRootIOCtor* )
   : TObject(),
     fOrigEvtNum(-1)
{

   fRWResults.clear();

}

void RWRecord::Reset()
{

   fOrigEvtNum=-1;
   fRWResults.clear();

   return;

}


void RWRecord::Copy( const RWRecord& rwrec )
{

   this->Reset();
   fOrigEvtNum    = rwrec.fOrigEvtNum; 
   fRWResults     = rwrec.fRWResults;
   
   return;

}

void RWRecord::Insert( const double twk, const double wt )
{
   
   fRWResults.push_back( RWInfo(twk,wt) );

   return;

}
