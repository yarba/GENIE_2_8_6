#include <TSystem.h>
#include <Riostream.h>

// Use GENIE message logger instead of std::cout/std::endl;
//
#include "Messenger/Messenger.h"

#include "validation/MINERvA/Common/ExpData.h"

using namespace PlotUtils;
using namespace genie;
using namespace genie::mc_vs_data;

//-------------------------------

ExpDataSet::~ExpDataSet()
{

   if ( fDataHisto )      delete fDataHisto;
   if ( fDataHistoShape ) delete fDataHistoShape;

}

MUH1D ExpDataSet::GetDataHistoAsShape() const
{

   // check if it comes with the exp. datasets
   // 
   if ( fDataHistoShape )
   {
      MUH1D h( *fDataHistoShape );        
      return h;
   }
   
   // if does not come with the exp.datasets make it up from what's available...
   // although it's still unclear how to handle sys errors, thus only stat errors
   // will be available in such case...
   //
   double total = 0.;
   
   for ( int i=1; i<=fDataHisto->GetNbinsX(); ++i )
   {
      total += fDataHisto->GetBinContent(i)*fDataHisto->GetBinWidth(i);
   }
   
   MUH1D h( *fDataHisto );
   h.Reset();
   for ( int i=1; i<=fDataHisto->GetNbinsX(); ++i )
   {
      h.SetBinContent( i, (fDataHisto->GetBinContent(i)*fDataHisto->GetBinWidth(i)/total) );
      h.SetBinError(   i, (fDataHisto->GetBinError(i)*fDataHisto->GetBinWidth(i)/total) );
   }
   
   return h;

}

bool ExpDataSet::Read( std::string path, Option_t*  )
{

   MINERvAExData::Read( path ); 
   
   CreateDataHisto();
   CreateDataHisto( true ); // shapeonly

   // now check if both corr.mtx and cov.mtx are NULL yet vesys is not empty
   // in this case you can do one of the following:
   // 1) add sys errors as a 2-universe error band - this way sys errors will 100% correlated
   // 2) add sys errors as a diagonal cov.matrix - this way the sys errors will be 100% UNcorrelated 
   //    with PushUncorrError( const std::string& name, TH1D* err ) 
   // 
   if ( fDataHisto && fSysErrExist )
   { 
      if ( fCorrMatrix == 0 && fCovMatrix == 0 )
      {
         // below is how to add 2-universe error band (100% corr.)
         // this is the easiest and most universal way to handle the case
         // as it fits both cases - symmetric or asymmetric errors
         //
         fDataHisto->AddVertErrorBand( "ESys", 2 );
         //
         // now you must reset it before filling up because by default it'll copy over
         // contents of fDataHisto into the C(entral) V(alue), thus if I fill it up
         // without resetting, your CV will double
         //
         //
         fDataHisto->GetVertErrorBand("ESys")->Reset();
         // 
         // OK, your CV is cleared, and you can safel fill your band
         //
         for ( int i=0; i<fDataHisto->GetNbinsX(); ++i )
         {
	    // fill up "vertical error band" with sys errors (use max spread, i.e. +/-sigma)
            // remember that bin *numbering* in TH starts at 1, not 0, this (i+1)th bin
	    // also remember to "weight" the bin by the xsec (vxsec[i]); otherwise the value will be 1
            // also recalculate sys errors (down/up) into **weights** (scale) that is (1.-/+(esys/xsec))
            //
            double xx = fDataHisto->GetBinCenter(i+1);
	    fDataHisto->FillVertErrorBand( "ESys", xx, 
                                           (1.-(fData[i].GetSysError()/fData[i].GetContent())), 
		   			   (1.+(fData[i].GetSysError()/fData[i].GetContent())), 
					   fData[i].GetContent() );      
         } 
      }     
   } // end-if check that neither corr.ntx nor cov.mtx exist

   //
   // now same thing for shape-only histo, if exists
   //
   if ( fDataHistoShape && fSysErrShapeExist )
   {
      if ( fCorrMatrixShape == 0 && fCovMatrixShape == 0 )
      {
         fDataHistoShape->AddVertErrorBand( "ESys_asShape", 2 );
	 fDataHistoShape->GetVertErrorBand( "ESys_asShape" )->Reset();
	 for ( int i=0; i<fDataHisto->GetNbinsX(); ++i )
	 {
            double xx = fDataHistoShape->GetBinCenter(i+1);
	    fDataHistoShape->FillVertErrorBand( "ESys_asShape", xx, 
                                                (1.-(fDataShape[i].GetSysError()/fDataShape[i].GetContent())), 
						(1.+(fDataShape[i].GetSysError()/fDataShape[i].GetContent())), 
						fDataShape[i].GetContent() );      	 
         }
      }
   }

   return true;

}

void ExpDataSet::CreateDataHisto( bool shapeonly ) 
{

   // could there be a situation when it needs to be replaced ??? 
   // I don't think so but...
   //
   if ( fDataHisto && !shapeonly )
   {
      LOG("gvldtest", pWARN) << " Data Histo already exists; do nothing" ;
      return;
   }
   
   if ( fDataHistoShape && shapeonly )
   {
      LOG("gvldtest", pWARN) << " Shape-Only Data Histo already exists; do nothing" ;
      return;   
   }

   // "waste" a bit of CPU and memory but it happens only once at constructions
   //
   std::vector<MINERvAExBin> dtmp;
   dtmp.clear();
   if ( shapeonly )
   {
      dtmp = fDataShape;
   }
   else
   {
      dtmp = fData;
   }

   int nxbins = dtmp.size();   
   double* xbins = new double[nxbins+1];
   
   for ( int i=0; i<nxbins; ++i )
   {
      xbins[i]   = dtmp[i].GetXMin();
      xbins[i+1] = dtmp[i].GetXMax();
   }
   
   // instantiate DataHisto with variable bin width
   // try to make its name unique
   // title is empty (at least, for now)
   //
   std::string hname = "DataHisto";
   if ( shapeonly ) hname += "Shape";
   std::string prj = fInteraction.GetProjectileName();
   std::string tg  = fInteraction.GetTgtNucleonName();
   hname += ( "_" + prj + "_" + tg + "_" + fObservable );
   if ( !shapeonly )
   {
      fDataHisto = new MUH1D(hname.c_str(),"", nxbins, xbins );
      fDataHisto->GetXaxis()->SetTitle( fObservable.c_str() );
      fDataHisto->GetXaxis()->SetTitleFont(62);
      fDataHisto->GetXaxis()->CenterTitle();
   }
   else
   {
      fDataHistoShape = new MUH1D(hname.c_str(),"", nxbins, xbins );
      fDataHistoShape->GetXaxis()->SetTitle( fObservable.c_str() );
      fDataHistoShape->GetXaxis()->SetTitleFont(62);
      fDataHistoShape->GetXaxis()->CenterTitle();
   }

   // fill up the DataHisto(Shape) - C(entral) V(alue) with stat error only, no sys error band(s) 
   //
   for ( int i=0; i<nxbins; ++i )
   {
      // calculate mid-bin
      //
      double xx = 0.5 * ( dtmp[i].GetXMin() + dtmp[i].GetXMax() );
      //
      // fill exp.data histo
      // remember to setup bin error (stat error on the xsec)
      //
      if ( !shapeonly )
      {
         int ibin = fDataHisto->Fill( xx, dtmp[i].GetContent() );  
         fDataHisto->SetBinError( ibin, dtmp[i].GetStatError() );
      }
      else
      {
         int ibin = fDataHistoShape->Fill( xx, dtmp[i].GetContent() );  
         fDataHistoShape->SetBinError( ibin, dtmp[i].GetStatError() );
      }
   }
   
   // add err. matrix
   //
   if ( !shapeonly )
   {
      if ( fCovMatrix ) 
      {
         fDataHisto->PushCovMatrix( "ESys", *fCovMatrix );
      } 
   }
   else
   {
      //
      // NOTE: there's a "feature" of in RooMUHisto that probably need to be fixed;
      //       only after that it can be fixed here.
      //       But for now I need to replicate the same matrix a) as regular sys err mtx 
      //       and b) as shape-only.
      //       The reason is because the (internal) search always relies on GetSysErrorMatricesNames() 
      //       that EXPLICITLY CHOPS OFF the "_asShape" variant !
      //       So without the "regular variant" the "_asShape" one is helpless/useless, helas !!!
      //
      if ( fCovMatrixShape )
      {
         fDataHistoShape->PushCovMatrix( "ESys", *fCovMatrixShape ); // 3rd arg - cov_area_normalize=true
         fDataHistoShape->PushCovMatrix( "ESys", *fCovMatrixShape, true ); // 3rd arg - cov_area_normalize=true
                                                                           // which means "_asShape"   
      }
   }
   
   delete [] xbins;

   return;

}


// -------------------------

ExpData::~ExpData()
{
   
   for ( size_t i=0; i<fExpDataSets.size(); ++i )
   {
      delete fExpDataSets[i];
   }
   fExpDataSets.clear();
   
}

void ExpData::ReadExpData( const std::string& path )
{

   bool exists = !( gSystem->AccessPathName( path.c_str() ) );
   if ( !exists )
   {
      LOG("gvldtest", pWARN) << " Exp.Data file is NOT found; do nothing" ;
      return;
   }

   fCurrentExpDataPath = path;
   
   AddExpDataSet( path );
   
   return;

}

void ExpData::AddExpDataSet( const std::string& path )
{

   fExpDataSets.push_back( new ExpDataSet() );
   
   bool status = (fExpDataSets.back())->Read( path );
   
   // check for invalid record
   //
   if ( !status ) 
   {
      LOG("gvldtest", pWARN) << " Corrupted Data Set: " << path << "; removing this record " ;
   }
   
   // TODO: also need to check if it's a duplicate !!!

   return ;
   
}

const ExpDataSet* ExpData::GetExpDataSet( const std::string& observable )
{

   for ( size_t i=0; i<fExpDataSets.size(); ++ i )
   {
//
// leftover from earlier round when the observable's name could have been "padded" with blank spaces at the start
//
//      if ( (fExpDataSets[i]->GetObservable()).find( observable ) != std::string::npos ) return fExpDataSets[i];   
      if ( fExpDataSets[i]->GetObservable() == observable ) return fExpDataSets[i];
   }
   
   return NULL;

} 
