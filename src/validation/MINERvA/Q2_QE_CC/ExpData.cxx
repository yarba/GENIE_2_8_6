#include <TSystem.h>
#include <Riostream.h>
#include <TDirectory.h>

// Use GENIE message logger instead of std::cout/std::endl;
//
#include "Messenger/Messenger.h"

#include "ExpData.h"
#include <algorithm> // to be able to use atof(...), etc.

using namespace PlotUtils;
using namespace genie;

//-------------------------------

ExpDataSet::~ExpDataSet()
{

   if ( fDataHisto )  delete fDataHisto;
   if ( fCorrMatrix ) delete fCorrMatrix;
   if ( fCovMatrix )  delete fCovMatrix;

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

bool ExpDataSet::ReadData( const std::string& path )
{

 
   bool exists = !( gSystem->AccessPathName( path.c_str() ) );
   if ( !exists )
   {
      LOG("gvldtest", pERROR)           
        << " Exp. data file not found in the given path " << path;
      return false;
   }

   fCurrentExpDataPath = path;
  
   ifstream infile1;
   infile1.open( path.c_str() );
   
   // double check if the exp. data file opens up fine
   // 
   if ( !(infile1.is_open()) )
   {
      LOG("gvldtest", pERROR)           
        << " Exp. data file in the given path " << path << " but can NOT be open" ;
      return false;
   }

   char line[256];
   for ( int i=0; i<256; ++i ) line[i] = ' ';
   std::vector<std::string> tokens;
   std::string del = " ";
   int counter = -1;

   std::vector<double> vxmin;
   std::vector<double> vxmax;
   std::vector<double> vdata;
   std::vector<double> vestat;
   std::vector<double> vesys;
   
   bool shapeflag = false;
   
   while ( !infile1.eof()  )
   {
      for ( int i=0; i<256; ++i ) line[i] = ' '; // cleanup the read-in buffer before next iteration

      infile1.getline( line, 256 );

      std::string line1 = std::string( line ); // , 256 );
      
      if ( line1.find("#") == 0 ) continue; // comment line
      if ( line1.find_first_not_of(del) == std::string::npos ) continue; // empty line
      
      if ( line1.find("<Reference>") != std::string::npos )
      {
         while (1)
	 {
	    for ( unsigned int i=0; i<line1.size(); ++i ) line[i] = ' ';
	    infile1.getline( line, 256  );
	    line1 = std::string( line );
	    if ( line1.find("#") == 0 ) continue; // comment line
	    if ( line1.find_first_not_of(del) == std::string::npos ) continue; // empty line
	    if ( line1.find("</Reference>") != std::string::npos ) break; // found end-tag </Observable>
	    fReference = line1;
         }
	 continue;
      }
      
      if ( line1.find("<Interaction>") != std::string::npos )
      {
         while(1)
	 {
	    for ( unsigned int i=0; i<line1.size(); ++i ) line[i] = ' ';
	    infile1.getline( line, 256  );
	    line1 = std::string( line );
	    if ( line1.find("#") == 0 ) continue; // comment line
	    if ( line1.find_first_not_of(del) == std::string::npos ) continue; // empty line
	    if ( line1.find("</Interaction>") != std::string::npos ) break; // found end-tag </Observable>
	    tokens.clear();
	    SplitString( line1, del, tokens );
	    if ( tokens.size() < 2 )
	    {
               LOG("gvldtest", pERROR)           
                  << " Invalid Interaction type";
	       return false;
	    }
	    bool status = fInteraction.SetType( tokens[0], tokens[1] );
	    if ( !status )
	    {
               LOG("gvldtest", pERROR)           
                  << " Invalid Interaction type";
	       return false;
	    }
	 }
      }
      
      if ( line1.find("<Observable>") != std::string::npos ) // found tag <Observable> 
      {
         while (1)
	 {
	    for ( unsigned int i=0; i<line1.size(); ++i ) line[i] = ' ';
	    infile1.getline( line, 256  );
	    line1 = std::string( line );
	    if ( line1.find("#") == 0 ) continue; // comment line
	    if ( line1.find_first_not_of(del) == std::string::npos ) continue; // empty line
	    if ( line1.find("</Observable>") != std::string::npos ) break; // found end-tag </Observable>
	    tokens.clear();
	    SplitString( line1, del, tokens );
	    if ( tokens.size() < 1 )
	    {
               LOG("gvldtest", pERROR)         
                  << " Observable is NOT specifid in the exp. data set";
	       return false;
	    }
	    fObservable = tokens[0];
	 }
	 continue;
      }
      
//      if ( line1.find("<DataTable>") != std::string::npos )
      if ( line1.find("<DataTable") != std::string::npos ) // do not include the ">" closing bracket
                                                           // since there maybe 2 tables - regilar and/or shape-only 
      {          	 
	 vxmin.clear();
	 vxmax.clear();
	 vdata.clear();
	 vestat.clear();
	 vesys.clear();
	 shapeflag = false;
	 if ( line1.find("ShapeOnly") != std::string::npos ) shapeflag = true;
	 while (1)
	 {
	    for ( unsigned int i=0; i<line1.size(); ++i ) line[i] = ' ';
	    infile1.getline( line, 256  );
	    line1 = std::string ( line );
	    if ( line1.find("#") == 0 ) continue; // comment line
	    if ( line1.find_first_not_of(del) == std::string::npos ) continue; // empty line
//	    if ( line1.find("</DataTable>") != std::string::npos ) break; // found end-tag for DataTable
	    if ( line1.find("</DataTable") != std::string::npos ) break; // found end-tag for DataTable
	                                                                 // NOTE: do not use the ">" closing bracket
									 // sincere there maybe 2 types - regular or shape-only
	    tokens.clear();
	    SplitString( line1, del, tokens );
	    if ( tokens.size() < 4 ) 
	    {
               LOG("gvldtest", pERROR)          
                  << " EMERGENCY !!! Insufficient Data Table \n " <<
		     " There must be at least xmin, xmax, data, and stat error on the data points";
	       return false;
	    }
	    vxmin.push_back( atof(tokens[0].c_str()) );
	    vxmax.push_back( atof(tokens[1].c_str()) );
	    vdata.push_back( atof(tokens[2].c_str()) );
	    vestat.push_back( atof(tokens[3].c_str()) );
	    if ( tokens.size() == 5 ) 
	    {
	          vesys.push_back( atof(tokens[4].c_str()) );
	    }
	 }
	 CreateDataHisto( vxmin, vxmax, vdata, vestat, shapeflag );
	 continue;
      }
      
//      if ( line1.find("<CorrelationMatrix>") != std::string::npos )
      if ( line1.find("<CorrelationMatrix") != std::string::npos ) // do not use the ">" closing braket 
                                                                    // since there maybe 2 types of corr.mtx
								    // - regular or shape-only
      {
	 
	 shapeflag = false;
	 if ( line1.find("ShapeOnly") != std::string::npos ) shapeflag = true;
	 
	 if ( fCorrMatrix && !shapeflag )
	 {
               LOG("gvldtest", pWARN)          
                  << " Correlation matrix is already defined !";
	    continue;
	 }
	 if ( fCorrMatrixShape && shapeflag )
	 {
               LOG("gvldtest", pWARN)          
                  << " Shape-only correlation matrix is already defined !";
	    continue;
	 }
	 
	 counter = 0;
	 while (1)
	 {
	    for ( unsigned int i=0; i<line1.size(); ++i ) line[i] = ' ';
	    infile1.getline( line, 256  );
	    line1 = std::string( line );	    
	    if ( line1.find("#") == 0 ) continue; // comment line
	    if ( line1.find_first_not_of(del) == std::string::npos ) continue; // empty line
//	    if ( line1.find("</CorrelationMatrix>") != std::string::npos ) break; // found end-tag for Corr.Mtx
	    if ( line1.find("</CorrelationMatrix") != std::string::npos ) break; // found end-tag for Corr.Mtx
	                                                                         // NOTE: do not use the ">" closing braket
	    tokens.clear();
	    SplitString( line1, del, tokens );
	    if ( tokens.size() != vdata.size() )
	    {
               LOG("gvldtest", pERROR)          
                  << " Matrix does not match the mnumber of data points !";
		  if ( fCorrMatrix ) delete fCorrMatrix;
		  fCorrMatrix = 0;
	          return false;
	    }
	    if ( !shapeflag ) 
	    {
	       // FIXME !!! it assumes that fDataHisto is already in place...
	       //
	       if ( !fCorrMatrix ) fCorrMatrix = new TMatrixD( (fDataHisto->GetNbinsX()+2), 
	                                                       (fDataHisto->GetNbinsX()+2) );
	       for ( unsigned int i=0; i<tokens.size(); ++i )
	       {
	          (*fCorrMatrix)[counter+1][i+1] = atof( tokens[i].c_str() ); // 0th and n-th bins are to handle
		                                                              // under/overflow of the MC histo
	       }
	    }
	    else if ( shapeflag )
	    {
	       if ( !fCorrMatrixShape ) fCorrMatrixShape = new TMatrixD( (fDataHistoShape->GetNbinsX()+2), 
	                                                                  (fDataHistoShape->GetNbinsX()+2) );
	       for ( unsigned int i=0; i<tokens.size(); ++i )
	       {
	          (*fCorrMatrixShape)[counter+1][i+1] = atof( tokens[i].c_str() ); // 0th and n-th bins are to handle
		                                                                   // under/overflow of the MC histo
	       }

	    }
	    counter++;
	 }
	 // here first check if data histo (or data histo shape-only) exists 
	 //
	 if ( !shapeflag ) // regular data table/matrix 
	 {
	    if ( !fDataHisto )
	    {
               LOG("gvldtest", pERROR)          
                  << " Something is wrong: there is corr.matrix but no data table/histo !!!";
	       if ( fCorrMatrix ) delete fCorrMatrix;
	       fCorrMatrix = 0;
	       return false;	       
	    }
	    if ( vesys.empty() )
	    {
	       // no sys errors, can't create cov.mtx; 
	       // destroy corr.mtx just in case a real cov.mtx is coming next from the exp. data file
	       //
	       if ( fCorrMatrix ) 
	       {
	          delete fCorrMatrix;
	          fCorrMatrix = 0;
	          continue;
	       }
	    }
	    if ( ((int)vesys.size()+2) != fCorrMatrix->GetNcols() )
	    {
	       // corr.mtx exists, and esys is also in
	       // BUT !!! they're of missmatching dimensions 
	       delete fCorrMatrix;
	       fCorrMatrix=0;
	       return false;
	    }
	 }
	 else // shape-only table/matrix
	 {
	    if ( !fDataHistoShape )
	    {
               LOG("gvldtest", pERROR)          
                  << " Something is wrong: there is shape-only corr.matrix but no shape-only data table/histo !!!";
	       if ( fCorrMatrixShape ) delete fCorrMatrixShape;
	       fCorrMatrixShape = 0;
	       return false;	       
	    }
	    if ( vesys.empty() )
	    {
	       // no sys errors, can't create cov.mtx; 
	       // destroy corr.mtx just in case a real cov.mtx is coming next from the exp. data file
	       //
	       if ( fCorrMatrixShape ) 
	       {
	          delete fCorrMatrixShape;
	          fCorrMatrixShape = 0;
	          continue;
	       }
	    }
	    if ( ((int)vesys.size()+2) != fCorrMatrixShape->GetNcols() )
	    {
	       // corr.mtx exists, and esys is also in
	       // BUT !!! they're of missmatching dimensions 
	       delete fCorrMatrixShape;
	       fCorrMatrixShape=0;
	       return false;
	    }
	 }	 
	 // OK, we get here if everything is fine
	 // now go ahead and create cov.mtx from corr.mtx & esys vector
	 //
	 CreateCovMatrix( vesys, shapeflag );
	 continue;
      }

// FIXME !!!
// Provisions for future development
/*
      if ( line1.find("<CovarianceMatrix>") != std::string::npos )
      {
         if ( fCovMatrix )
	 {
	    std::cout << " Covariance matrix is already defined ! " << std::endl;
	    continue;
	 }
      }      
*/      
   }
   
   infile1.close();
         
   // now check if both corr.mtx and cov.mtx are NULL yet vesys is not empty
   // in this case you can do one of the following:
   // 1) add sys errors as a 2-universe error band - this way sys errors will 100% correlated
   // 2) add sys errors as a diagonal cov.matrix - this way the sys errors will be 100% UNcorrelated 
   //    with PushUncorrError( const std::string& name, TH1D* err ) 
   // 
   if ( !shapeflag && fDataHisto && fCorrMatrix == 0 && fCovMatrix == 0 )
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
                                        (1.-(vesys[i]/vdata[i])), (1.+(vesys[i]/vdata[i])), vdata[i] );      
      }      
      //
      // this commented out example shows how to put in uncorr. errors
      //
      // int nxbins = vxmin.size();
      // double* xbins = new double[nxbins+1];
      // for ( int i=0; i<nxbins; ++i )
      // {
      //   xbins[i]   = vxmin[i];
      //   xbins[i+1] = vxmax[i];
      // }
      // TH1D* hsyserr = new TH1D("hsyserr","hsyserr",nxbins,xbins);
      // for ( int i=0; i<nxbins; ++i )
      // {
      //   double xx = 0.5*(vxmin[i]+vxmax[i]);
      //	 hsyserr->Fill( xx, vesys[i] );
      //}
      // fDataHisto->PushUncorrError( "ESys", hsyserr );
      // delete [] xbins;
   } // end-if check that neither corr.ntx nor cov.ntx exist

   //
   // now same thing for shape-only histo, if exists
   //
   if ( fDataHistoShape && shapeflag )
   {
      if ( fCorrMatrixShape == 0 && fCovMatrixShape == 0 )
      {
         fDataHistoShape->AddVertErrorBand( "ESys_asShape", 2 );
	 fDataHistoShape->GetVertErrorBand( "ESys_asShape" )->Reset();
	 for ( int i=0; i<fDataHisto->GetNbinsX(); ++i )
	 {
            double xx = fDataHistoShape->GetBinCenter(i+1);
	    fDataHistoShape->FillVertErrorBand( "ESys_asShape", xx, 
                                                (1.-(vesys[i]/vdata[i])), (1.+(vesys[i]/vdata[i])), vdata[i] );      	 
         }
      }
   }

   return true;

}

void ExpDataSet::CreateDataHisto( const std::vector<double>& vxmin, const std::vector<double>& vxmax, 
                                  const std::vector<double>& vdata, const std::vector<double>& vestat,
				  bool shapeonly )
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


   int nxbins = vxmin.size();
   double* xbins = new double[nxbins+1];
   for ( int i=0; i<nxbins; ++i )
   {
      xbins[i]   = vxmin[i];
      xbins[i+1] = vxmax[i];
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

   // fill up the DataHisto - C(entral) V(alue) with stat error only, no sys error band(s) 
   //
   for ( int i=0; i<nxbins; ++i )
   {
      // calculate mid-bin
      //
      double xx = 0.5 * ( vxmin[i] + vxmax[i] );
      //
      // fill exp.data histo
      // remember to setup bin error (stat error on the xsec)
      //
      if ( !shapeonly )
      {
         int ibin = fDataHisto->Fill( xx, vdata[i] );  
         fDataHisto->SetBinError( ibin, vestat[i] );
      }
      else
      {
         int ibin = fDataHistoShape->Fill( xx, vdata[i] );  
         fDataHistoShape->SetBinError( ibin, vestat[i] );
      }
   }
   
   delete [] xbins;

   return;

}

void ExpDataSet::CreateCovMatrix( const std::vector<double>& vesys, bool shapeonly )
{

   // make sure corr.mtx object(s) exists
   //
   // FIXME !!! Spit out a warning message ???
   //
   if ( !fCorrMatrix && !shapeonly ) return;
   
   if ( !fCorrMatrixShape && shapeonly ) return;
   
   // convert the vector of esys (as per bin) into diagonal mtx
   int nx = vesys.size();
   TMatrixD ESys(nx+2,nx+2);
   for ( int i=0; i<nx; ++i )
   {
      ESys[i+1][i+1] = vesys[i];
   }
   
  // FIXME !!!
   // The MINERvA paper says the corr.mtx is for *total* (stat+sys) uncertainties
   // should I add stat & sys in quadrature, and then apply the corr.mtx ????
   //
   // for now let's do sys errors only
   //
   
   // Cov = ESys * Corr * ESys 
   //
   if ( !shapeonly )
   {
      fCovMatrix = new TMatrixD(nx+2,nx+2);
      (*fCovMatrix) = ESys * (*fCorrMatrix);
      (*fCovMatrix) *= ESys;   
      // and now push it together with the data histo
      //
      fDataHisto->PushCovMatrix( "ESys", *fCovMatrix ); 
   }
   else
   {
      fCovMatrixShape = new TMatrixD(nx+2,nx+2);
      (*fCovMatrixShape) = ESys * (*fCorrMatrixShape);
      (*fCovMatrixShape) *= ESys;

      //
      // FIXME !!! (in RooMUHisto,... and only after that here)
      // I need to replicate the same matrix a) as regular sys err mtx & b) as shape-only
      // The reason is because the (internal) search always relies on GetSysErrorMatricesNames() method
      // that EXPLICITLY CHOPS OFF the "_asShape" variant !
      // So without the "regular variant" the "_asShape" one is helpless/useless, helas !!!
      //
      fDataHistoShape->PushCovMatrix( "ESys", *fCovMatrixShape ); // 3rd arg - cov_area_normalize=true
      fDataHistoShape->PushCovMatrix( "ESys", *fCovMatrixShape, true ); // 3rd arg - cov_area_normalize=true
                                                                        // which means "_asShape"   
   }
      
   return;

}

void ExpDataSet::SplitString( const std::string& str, const std::string del, std::vector<std::string>& tokens )
{

   string::size_type last = str.find_first_not_of(del, 0);
   string::size_type pos  = str.find_first_of(del, last);
   while (std::string::npos != pos || std::string::npos != last)
   {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(last, pos-last));
      // Skip delimiters.  Note the "not_of"
      last = str.find_first_not_of(del, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(del, last);
   }

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

   // this actually has to a be an XML file that could specify multiple datasets !!!

   bool exists = !( gSystem->AccessPathName( path.c_str() ) );
   if ( !exists )
   {
      std::cout << " exp. data file is not found " << std::endl;
      return;
   }

   fCurrentExpDataPath = path;
   
   AddExpDataSet( path );
   
   return;

}

void ExpData::AddExpDataSet( const std::string& path )
{

   fExpDataSets.push_back( new ExpDataSet() );
   
   bool status = (fExpDataSets.back())->ReadData( path );
   
   // invalid record
   //
   if ( !status ) 
   {
      std::cout << " Corrupted Data Set: " << path << "; removing this record " << std::endl; 
   }
   
   // also need to check if it's a duplicate !!!

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
