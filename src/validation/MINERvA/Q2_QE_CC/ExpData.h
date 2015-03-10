#ifndef Val_MINERvA_ExpData_H
#define Val_MINERvA_ExpData_H

#include <vector>
#include <string>
#include <iostream>

#include "PlotUtils/MUH1D.h"

#include "validation/MINERvA/DataComp/IntType.h"
#include "validation/MINERvA/DataComp/MINERvAExData.h"

class ExpDataSet : public genie::mc_vs_data::MINERvAExData
{

   public:
   
      // ctor & dtor
      //
      ExpDataSet() : genie::mc_vs_data::MINERvAExData(),
                     fDataHisto(0), 
		     fDataHistoShape(0) {};
      virtual ~ExpDataSet();
      
      const std::string&  GetReference()         const { return fReference; }
      const std::string&  GetObservable()        const { return fObservable; }
      const IntType&      GetInteraction()       const { return fInteraction; }

// Don't give a bare pointer anymore
//
// -->      PlotUtils::MUH1D*   GetDataHistoPtr() const { return fDataHisto; }
      PlotUtils::MUH1D    GetDataHisto()          const { PlotUtils::MUH1D h(*fDataHisto); return h;  }
      PlotUtils::MUH1D    GetDataHistoAsShape()   const;    
      
      virtual bool Read( const std::string path, Option_t* opt=0 ) ;
           
   protected: 
   
      virtual void CreateCovMatrix(  bool shapeonly=false );
   
   private:
   
      // member functions
      //
//      void CreateDataHisto( const std::vector<double>&, const std::vector<double>&, 
//                            const std::vector<double>&, const std::vector<double>&,
//			    bool shapeonly=false );
      void CreateDataHisto( bool shapeonly=false );
   
      // data members
      //            
      PlotUtils::MUH1D* fDataHisto;      
      PlotUtils::MUH1D* fDataHistoShape;

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
