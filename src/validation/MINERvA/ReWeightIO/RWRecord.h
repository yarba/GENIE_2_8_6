#ifndef RWRECORD_H_
#define RWRECORD_H_

// #include <ostream>
// #include <string>
#include <vector>

#include <TObject.h>

class TRootIOCtor;

class RWInfo : public TObject {

   public:
   
      RWInfo() : TObject(), fTweak(0.), fWeight(1.) {}
      RWInfo( double twk, double wt ) : TObject() { fTweak=twk; fWeight=wt; }
      RWInfo( TRootIOCtor* ) : TObject(), fTweak(0.), fWeight(1.) {}
      RWInfo( const RWInfo& r ) { fTweak=r.fTweak; fWeight=r.fWeight; }
      ~RWInfo() {}
      
      double GetTweak()  const { return fTweak; }
      double GetWeight() const { return fWeight; }

   private:
      
      double fTweak;
      double fWeight;
      
// NOTE (JY): the 2nd parameter does have a "sense" of versioning, but it's more than that;
//            in particular, 2 is the "minimal level" for reasonable I/O (e.g.0 would mean 
//            "never store it");
//            one would increase it when chaning/adding to the data member list, etc.
//
ClassDef(RWInfo,2)

};


class RWRecord : public TObject {

   public:
   
/*
// NOTE (JY): (Part of) The reason to make it into a separate class is that 
//            that with the embedded class implementation I haven't managed 
//            to read it back in properly
//
      class RWInfo {

         public:
   
            RWInfo() : fTweak(0.), fWeight(1.) {}
            RWInfo( double twk, double wt ) { fTweak=twk; fWeight=wt; }
            RWInfo( TRootIOCtor* ) : fTweak(0.), fWeight(1.) {}
            RWInfo( const RWInfo& r ) { fTweak=r.fTweak; fWeight=r.fWeight; }
            ~RWInfo() {}
      
            double GetTweak()  const { return fTweak; }
            double GetWeight() const { return fWeight; }

         private:
      
            double fTweak;
            double fWeight;
   
      };
*/      
      RWRecord();
      RWRecord( const RWRecord& );
      RWRecord( TRootIOCtor* );
      
      ~RWRecord() {}
      
      void Reset();
      void Copy( const RWRecord& );
             
      void SetOriginalEvtNumber( const int ievt ) { fOrigEvtNum = ievt; return; }
      void Insert( const double, const double );
      
      int    GetOriginalEvtNumber()   const { return fOrigEvtNum; }
      int    GetNumOfRWResults()      const { return fRWResults.size(); }
      double GetTweak( const int i )  const { return ( ( fRWResults.size() > 0 && i >= 0 && i < (int)fRWResults.size() ) ? fRWResults[i].GetTweak() : 0. ); }
      double GetWeight( const int i ) const { return ( ( fRWResults.size() > 0 && i >= 0 && i < (int)fRWResults.size() ) ? fRWResults[i].GetWeight() : 1. ); }

   private:
                  
      int                 fOrigEvtNum;
      std::vector<RWInfo> fRWResults;
      
ClassDef(RWRecord,2)

};

#endif // RWOBJECT_H_
