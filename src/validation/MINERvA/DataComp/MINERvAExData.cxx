#include <TSystem.h>
#include <Riostream.h>
#include <TDirectory.h>

// Use GENIE message logger instead of std::cout/std::endl;
//
#include "Messenger/Messenger.h"

#include "validation/MINERvA/DataComp/MINERvAExData.h"
//#include "TMatrixD.h"
//#include "TMatrixDBase.h"
#include <algorithm> // to be able to use atof(...), etc.

using namespace genie;
using namespace genie::tune;
using namespace genie::mc_vs_data;

//-------------------------------

MINERvAExData::~MINERvAExData()
{

   if ( fCorrMatrix ) delete fCorrMatrix;
   if ( fCovMatrix )  delete fCovMatrix;
   if ( fCorrMatrixShape ) delete fCorrMatrixShape;
   if ( fCovMatrixShape)   delete fCovMatrixShape;

}

//----------?????????? NEW DEV !!!!!!!!!!!!!!!

bool MINERvAExData::Read( const std::string path, Option_t* )
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

	    double esys = 0.; 	// default    
            if ( tokens.size() == 5 )
	    {
	       //
	       // the 1st line defines if sys err should be there or not
	       //
	       if ( shapeflag )
	       {
	          if ( fDataShape.empty() )
		  {
		     if ( !fSysErrShapeExist ) fSysErrShapeExist = true;
		     if ( fSysErrShapeExist ) esys = atof(tokens[4].c_str());
		  }
	       }
	       else
	       {
	          if ( fData.empty() )
		  {
		     if ( !fSysErrExist ) fSysErrExist = true;
		     if ( fSysErrExist ) esys = atof(tokens[4].c_str());
		  }
	       }
	    }
	    	    
	    if ( shapeflag )
	    {
	       fDataShape.push_back( MINERvAExBin( atof( tokens[0].c_str()), atof(tokens[1].c_str()), 
	                                           atof(tokens[2].c_str()), atof(tokens[3].c_str()),
				                   esys ) );
	    }
	    else
	    {
	       fData.push_back( MINERvAExBin( atof( tokens[0].c_str()), atof(tokens[1].c_str()), 
	                                      atof(tokens[2].c_str()), atof(tokens[3].c_str()),
				              esys ) );
	    }
	 }
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
	    if ( tokens.size() != fData.size() )
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
	       if ( !fCorrMatrix ) fCorrMatrix = new TMatrixD( ((int)(fData.size())+2), 
	                                                       ((int)(fData.size())+2) );
	       for ( unsigned int i=0; i<tokens.size(); ++i )
	       {
	          (*fCorrMatrix)[counter+1][i+1] = atof( tokens[i].c_str() ); // 0th and n-th bins are to handle
		                                                              // under/overflow of the MC histo
	       }
	    }
	    else if ( shapeflag )
	    {
	       if ( !fCorrMatrixShape ) fCorrMatrixShape = new TMatrixD( ((int)(fDataShape.size())+2), 
	                                                                 ((int)(fDataShape.size())+2) );
	       for ( unsigned int i=0; i<tokens.size(); ++i )
	       {
	          (*fCorrMatrixShape)[counter+1][i+1] = atof( tokens[i].c_str() ); // 0th and n-th bins are to handle
		                                                                   // under/overflow of the MC histo
	       }

	    }
	    counter++;
	 }
	 // here first check if data (or data-shape-only) exists 
	 //
	 if ( !shapeflag ) // regular data table/matrix 
	 {
	    if ( fData.empty() )
	    {
               LOG("gvldtest", pERROR)          
                  << " Something is wrong: there is corr.matrix but no data table/histo !!!";
	       if ( fCorrMatrix ) delete fCorrMatrix;
	       fCorrMatrix = 0;
	       return false;	       
	    }
	    if ( !fSysErrExist )
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
	 }
	 else // shape-only table/matrix
	 {
	    if ( fDataShape.empty() )
	    {
               LOG("gvldtest", pERROR)          
                  << " Something is wrong: there is shape-only corr.matrix but no shape-only data table/histo !!!";
	       if ( fCorrMatrixShape ) delete fCorrMatrixShape;
	       fCorrMatrixShape = 0;
	       return false;	       
	    }
	    if ( !fSysErrShapeExist )
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
	 }	 
	 // OK, we get here if everything is fine
	 // now go ahead and create cov.mtx from corr.mtx & esys vector
	 //
//	 CreateCovMatrix( vesys, shapeflag ); // -------------> FIXME !!!
	 CreateCovMatrix( shapeflag ); 
	 continue;
      }

// FIXME !!!
// Provisions for future development
//
//      if ( line1.find("<CovarianceMatrix>") != std::string::npos )
//      {
//         if ( fCovMatrix )
//	 {
//	    std::cout << " Covariance matrix is already defined ! " << std::endl;
//	    continue;
//	 }
//      }      
//      
   }
   
   infile1.close();
         
   return true;

}

void MINERvAExData::CreateCovMatrix( bool shapeonly )
{
   
   // make sure corr.mtx object(s) exists
   //
   // FIXME !!! Spit out a warning message ???
   //
   if ( !fCorrMatrix && !shapeonly ) return;
   
   if ( !fCorrMatrixShape && shapeonly ) return;
   
   int nx = 0;
   if ( shapeonly )
   {
      nx = fDataShape.size();
   }
   else
   {
      nx = fData.size();
   }
   if ( nx == 0 ) return; // FIXME !!! need to spit a warning message !!!
   TMatrixD ESys(nx+2,nx+2);
   for ( int i=0; i<nx; ++i )
   {
      if ( shapeonly )
      {
         ESys[i+1][i+1] = fDataShape[i].GetSysError();
      }
      else
      {
         ESys[i+1][i+1] = fData[i].GetSysError();
      }
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
   }
   else
   {
      fCovMatrixShape = new TMatrixD(nx+2,nx+2);
      (*fCovMatrixShape) = ESys * (*fCorrMatrixShape);
      (*fCovMatrixShape) *= ESys;
   }
      
   return;

}

void MINERvAExData::SplitString( const std::string& str, const std::string del, std::vector<std::string>& tokens )
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

