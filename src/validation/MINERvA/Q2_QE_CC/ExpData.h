#ifndef Val_MINERvA_ExpData_H
#define Val_MINERvA_ExpData_H

#include <vector>
#include <string>
#include <iostream>

#include "PlotUtils/MUH1D.h"

#include "IntType.h"

class ExpDataSet
{

   public:
   
      // ctor & dtor
      //
      ExpDataSet() : fCurrentExpDataPath(""), fReference(""), fObservable(""), 
                     fDataHisto(0), fCorrMatrix(0), fCovMatrix(0),
		     fDataHistoShape(0), fCorrMatrixShape(0), fCovMatrixShape(0) {};
      ~ExpDataSet();
      
      const std::string&  GetReference()         const { return fReference; }
      const std::string&  GetObservable()        const { return fObservable; }
      const IntType&      GetInteraction()       const { return fInteraction; }

// Don't give a bare pointeranymore
//
// -->      PlotUtils::MUH1D*   GetDataHistoPtr() const { return fDataHisto; }
      PlotUtils::MUH1D    GetDataHisto()          const { PlotUtils::MUH1D h(*fDataHisto); return h;  }
      PlotUtils::MUH1D    GetDataHistoAsShape()   const;    
      
      bool ReadData( const std::string& path ) ;
           
   private:
   
      // member functions
      //
      void CreateDataHisto( const std::vector<double>&, const std::vector<double>&, 
                            const std::vector<double>&, const std::vector<double>&,
			    bool shapeonly=false );
      void CreateCovMatrix( const std::vector<double>&, bool shapeonly=false );
      void SplitString( const std::string&, const std::string, std::vector<std::string>& );
   
      // data members
      //
      std::string       fCurrentExpDataPath;
      
      std::string       fReference;
      
      IntType           fInteraction;
      std::string       fObservable;
      
      PlotUtils::MUH1D* fDataHisto;
      TMatrixD*         fCorrMatrix;
      TMatrixD*         fCovMatrix;
      
      PlotUtils::MUH1D* fDataHistoShape;
      TMatrixD*         fCorrMatrixShape;
      TMatrixD*         fCovMatrixShape;   

};


class ExpData
{

   public:
   
      // ctor & dtor
      //
      ExpData() : fCurrentExpDataPath("") {};
      ~ExpData();
      
      // read-in exp.data (given /path/to/data on input)
      //
      void ReadExpData( const std::string& );
      
      // access methods
      //
      int GetNDataSets() const { return fExpDataSets.size(); }
      const ExpDataSet* GetExpDataSet( int i ) const { return fExpDataSets[i]; } 
      const ExpDataSet* GetExpDataSet( const std::string& ); 

   private:
   
      void AddExpDataSet( const std::string& path );

      std::string       fCurrentExpDataPath;
      std::vector< ExpDataSet* > fExpDataSets;

};

#endif
