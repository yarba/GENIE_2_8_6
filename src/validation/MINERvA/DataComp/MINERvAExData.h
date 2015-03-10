#ifndef MINERvAExData_H
#define MINERvAExData_H

#include <vector>
#include <string>
#include <iostream>

#include "TObject.h"
#include "TMatrixD.h"
#include "TMatrixDBase.h"
class TRootIOCtor;

#include "validation/Tuning/GExDataABC.h"
#include "validation/MINERvA/DataComp/IntType.h"

// fwd declaration
namespace genie
{
   namespace tune
   {
      class GExModelABC;
   }
}

namespace genie
{
namespace mc_vs_data
{

class MINERvAExBin : public TObject
{

   public:
      
      // ctor & dtor
      //
      MINERvAExBin() : TObject(), fXMin(0.), fXMax(0.), fContent(0.), fStatErr(0.), fSysErr(0.) {}
      MINERvAExBin( double xmin, double xmax, double y, double estat, double esys ) : TObject() { fXMin=xmin;
                                                                                                  fXMax=xmax;
											          fContent=y;
											          fStatErr=estat;
											          fSysErr=esys; }
      MINERvAExBin( TRootIOCtor* ) : TObject(), fXMin(0.), fXMax(0.), fContent(0.), fStatErr(0.), fSysErr(0.) {}
      MINERvAExBin( const MINERvAExBin& b ): TObject() { fXMin=b.fXMin; fXMax=b.fXMax; 
                                                         fContent=b.fContent; fStatErr=b.fStatErr; fSysErr=b.fSysErr; }
      ~MINERvAExBin() {}
      
      // getters
      //
      double GetXMin()      const { return fXMin; }
      double GetXMax()      const { return fXMax; }
      double GetContent()   const { return fContent; }
      double GetStatError() const { return fStatErr; }
      double GetSysError()  const { return fSysErr; }
   
   protected:
     
        double fXMin;
	double fXMax;
	double fContent;
	double fStatErr;
	double fSysErr;

ClassDef(MINERvAExBin,2)

};

}
}

namespace genie
{
namespace mc_vs_data
{
class MINERvAExData : public genie::tune::GExDataABC
{

   public:
   
      // ctor & dtor
      //
      MINERvAExData() : genie::tune::GExDataABC(), 
                        fCurrentExpDataPath(""), fReference(""), fObservable(""), 
			fSysErrExist(false), 
			fCorrMatrix(0), fCovMatrix(0),
			fSysErrShapeExist(false),
			fCorrMatrixShape(0), fCovMatrixShape(0) {};
      virtual ~MINERvAExData();
      
      // should it be const ???
      //
      virtual std::string Name() { return "MINERvAExData"; }      
      
      virtual bool Read( std::string path, Option_t* opt=0 );

      // Mmmmm... not sure what's the purpose of this...
      //
      virtual bool Generate( genie::tune::GExModelABC* model, const genie::tune::GFitParamList& fp, bool fluctuate=true ) { return true; }

      // FIXME !!!
      // Here I conform to the GExDataABC interface but... shouldn't these methods below be const ???
      //
      virtual bool Write( TObjArray* collection, const char* tag = "data", Option_t* write_opt=0 ) { return true; }
      virtual bool Write( const char* fname,     const char* tag = "data", Option_t* write_opt=0, Option_t* file_opt=0 ) { return true; }
      #      
      virtual int NumOfBins() { return (int)(fData.size()); } 
      
      virtual double Statistic( genie::tune::GExModelABC* pred, const genie::tune::GFitParamList& fp, Option_t* stat_opt="likelihood-ratio" ) {return 0.; }
      virtual double Statistic( genie::tune::GExModelABC* pred, Option_t* stat_op="likelihood-ratio" ) { return 0.; }
                      
   protected:
   
      // member functions
      //
      void CreateCovMatrix( bool shapeonly=false );
      void SplitString( const std::string&, const std::string, std::vector<std::string>& );
   
      // data members
      //
      std::string         fCurrentExpDataPath;
      
      std::string         fReference; // i.e. journal/publication      
      IntType             fInteraction;
      std::string         fObservable;
      
      std::vector<MINERvAExBin>  fData;
      bool                fSysErrExist;
      TMatrixD*           fCorrMatrix;
      TMatrixD*           fCovMatrix;
      
      std::vector<MINERvAExBin> fDataShape;
      bool                fSysErrShapeExist;
      TMatrixD*           fCorrMatrixShape;
      TMatrixD*           fCovMatrixShape;   

};

} // end of mc_vs_data namespace

} // end of genie namespace


#endif
