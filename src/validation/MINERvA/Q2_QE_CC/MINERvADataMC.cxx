
#include "MINERvADataMC.h"

#include "ExpData.h"
#include "BaseAnalyzer.h"

#include "PlotUtils/MUH1D.h"
// #include "PlotUtils/MUApplication.h"
#include "PlotUtils/MUPlotter.h"


using namespace PlotUtils;
//using namespace genie;

MINERvADataMC::MINERvADataMC()
   : ResultsDataMC()
{

   // RooMUHistos example(s) also shows use of the Initialize() function
   // (see MUApplication.cxx), which will actually enable ROOT::Cintex
   // that might be needed for ROOT I/O, etc.
   // But it doesn't seem to be necessary for this particular app. 
   
   fPlotter = new MUPlotter();

   // NOTE: the exp. data table comes already "bin-normalized"
   //       for this reason the MC histo is also bin-normalized
   //       turn off bin normalization feature; otherwise it'll be normalized twice !!!
   //
   fPlotter->draw_normalized_to_bin_width = false;

}

MINERvADataMC::~MINERvADataMC()
{
   if ( fPlotter ) delete fPlotter;
}

void MINERvADataMC::DoIt( const ExpDataSet* dset, const BaseAnalyzer* analyzer )
{

   MUH1D dhisto   = dset->GetDataHisto();
   MUH1D simhisto = analyzer->GetSimHistoFluxNormalized(); 
   
   std::string proj       = (dset->GetInteraction()).GetProjectileName();
   std::string hitnucl    = (dset->GetInteraction()).GetTgtNucleonName();
   std::string observable = dset->GetObservable();
   
   TCanvas* cnv1 = new TCanvas("cnv1","",800.,600.);
   
   //
   // plot Data & MC
   //
   fPlotter->DrawDataMCWithErrorBand( &dhisto, &simhisto, 1., "R" ); // 1=scale, "R"="legend of the right"
   fPlotter->AddChi2Label( &dhisto, &simhisto, "TR", true ); 
   //
   // form & add title for the plot
   //
   std::string plottitle = proj + " on " + hitnucl + " (MINERvA & GENIE)";
   double titlesize = 0.05; // by default, MuPlotter::title_size=0.06
   fPlotter->AddHistoTitle( plottitle.c_str(), titlesize ); 
   
   std::string outname = "MINERvA-" + proj + "-" + hitnucl + "-" + observable + "-1";
   fPlotter->MultiPrint( cnv1, outname.c_str(), "gif" );
   
   //
   // plot Data and MC, including EACH variant from the band (re-weighting)
   // do it "manually", just to illustrate how it can be done
   //
   TCanvas* cnv2 = new TCanvas("cnv2","",800.,600.);
   TH1D dtmp = dhisto.GetCVHistoWithError(true);
   dtmp.SetMarkerStyle(20);
   dtmp.SetMarkerSize(1.0);
   dtmp.SetMarkerColor(kBlack);
   cnv2->cd();
   dtmp.Draw("p E1 X0");
   simhisto.SetLineColor(kRed);
   simhisto.SetLineWidth(3);
   simhisto.Draw("histsame");
   //
   // form, position, add various labels (e.g. chi2 for CV and different variants from re-weighting)
   //
   double xpos, ypos;
   double yoffset = 0.;
   int align = 22; // copied over from RooMUHistos/MUPlotter default
   fPlotter->DecodePosition( "TR", fPlotter->legend_text_size, align, xpos, ypos );
   fPlotter->AddPlotLabel( "Use Data Errors", xpos, ypos, fPlotter->legend_text_size, 1, 62, align );
   yoffset +=  fPlotter->legend_text_size;
   fPlotter->AddPlotLabel( "GENIE original", xpos, ypos-yoffset, fPlotter->legend_text_size, 1, 62, align );
   yoffset +=  fPlotter->legend_text_size;
   fPlotter->AddChi2Label( &dhisto, &simhisto, 1.0, "TR", fPlotter->legend_text_size, -yoffset, true );
   //
   // Now extract the histo variants that form the vert band (from re-weighting on Ma)
   // draw them, calculate chi2, etc.
   //
   // In principle, one can acheive (about) the same drawing with simband->DrawAll("hist",true)
   // but in such case the MUPlotter will contril the line collor, line style, etc.
   // Obviously, in order to do chi2 calculates for each variant, one needs to extract it anyway
   //
   // NOTE: the name Mass-CC-QE is hardcoded for now, and it's probably a tmp name
   //       in GENIE, this parameter is called MaCCQE (axial mass QE CC)
   //       in the re-weighting procedure via grwght1scan it becomes, bt default, embedded 
   //       in the name of the output file (unless a user specifies a output name)
   //       however, it doesn't seem to be present anywhere *inside* the reweighting's output 
   //
   if ( simhisto.HasVertErrorBand( "Reweight-Mass-CCQE" ) )
   {
         const MUVertErrorBand* simband = simhisto.GetVertErrorBand( "Reweight-Mass-CCQE" );
	 int nbandhi = simband->GetNHists();
	 if ( nbandhi > 0 ) 
	 {
	    yoffset += ( fPlotter->legend_text_size + 0.01 );
	    fPlotter->AddPlotLabel( "Reweight-Mass-CCQE Variants", xpos, (ypos-yoffset), 
	                            fPlotter->legend_text_size, 1, 62, align ); 
	    for ( int ih=0; ih<nbandhi; ++ih )
	    {
	       const TH1D* bandhi = simband->GetHist( ih );
	       MUH1D bandtmp( *bandhi );
	       bandtmp.SetLineColor(kBlack+ih+1);
	       bandtmp.SetLineStyle(2);
	       bandtmp.SetLineWidth(3);
	       bandtmp.DrawCopy("histsame");
	       yoffset += fPlotter->legend_text_size;
	       fPlotter->AddChi2Label( &dhisto, &bandtmp, 1.0, "TR", fPlotter->legend_text_size, -yoffset, true );
	    }
         }
   }      
   fPlotter->AddHistoTitle( plottitle.c_str(), titlesize ); 
   //
   // print out cnv2
   //
   outname = "MINERvA-" + proj + "-" + hitnucl + "-" + observable + "-2";
   // outname = proj + "-" + hitnucl + "-2";
   fPlotter->MultiPrint( cnv2, outname.c_str(), "gif" );
   
   //
   // plot as Data/MC + chi2/ndf calculated over the sim CV
   //
   TCanvas* cnv3 = new TCanvas("cnv3","",800.,600.);
   gPad->SetLogx();
   fPlotter->DrawDataMCRatio( &dhisto, &simhisto );
   fPlotter->AddChi2Label( &dhisto, &simhisto, "TC" ); // BL==top-right, TC==top-center 
                                                       // the 4th arg is omited, by default it's false and means "use MC errors"
						       // while true would mean "use data errors"
      
   fPlotter->DecodePosition( "TC", fPlotter->legend_text_size, align, xpos, ypos );
   fPlotter->AddPlotLabel( "Note: Use MC Errors", xpos, (ypos-2.*fPlotter->legend_text_size), 
                           fPlotter->legend_text_size, 1, 62, align );
   //
   // print out cnv3
   //
   outname = "MINERvA-" + proj + "-" + hitnucl + "-" + observable + "-3";
   // outname = proj + "-" + hitnucl + "-3";
   fPlotter->MultiPrint( cnv3, outname.c_str(), "gif" );
   
   TCanvas* cnv4 = new TCanvas("cnv4","",800,600);
   MUH1D simshape = analyzer->GetSimHistoAsShape();
   MUH1D dshape   = dset->GetDataHistoAsShape();
            
//      fPlotter->DrawDataMCWithErrorBand( &dshape, &simshape, 1., "TR" ); // this plots WITHOUT normalizing the band
                                                                          // on the shape-only MC histo...
									  //
   fPlotter->DrawDataMCWithErrorBand( &dshape, &simshape, 1., "TR", false, NULL, NULL, true ); // this will "normalize"
                                                                                               // the band on the shape-only MC histo...
											       // ... although I still don't get why 
											       // it'd be needed at all...
											       //
//      fPlotter->AddChi2Label( &dshape, &simshape, "R", false, true ); // this uses errors on the MC in the shape-only chi2 calculations
   fPlotter->AddChi2Label( &dshape, &simshape, "R", true, true );    // this uses data errors on the shape-only chi2 calculations
   plottitle = proj + " on " + hitnucl + " (MINERvA and GENIE - shape-only histo)";
   fPlotter->AddHistoTitle( plottitle.c_str(), 0.035 ); // 2nd arg is the plot text size
   //
   // print out cnv4
   //
   outname = "MINERvA-" + proj + "-" + hitnucl + "-" + observable + "-4";
   // outname = proj + "-" + hitnucl + "-4";
   fPlotter->MultiPrint( cnv4, outname.c_str(), "gif" );
   
   delete cnv1;
   delete cnv2;
   delete cnv3;
   delete cnv4;
      
   return;

}

void MINERvADataMC::ServicePrint( const MUH1D* htst ) const
{

// Below are some test printouts

   std::vector<std::string> errNames = htst->GetErrorBandNames();
   std::cout << "MUErrorBand names: " << std::endl;
   for( std::vector<std::string>::iterator it = errNames.begin(); it != errNames.end(); ++it )
   {
      std::cout << "         name = " << *it << std::endl;
      std::cout << *it << " has " << htst->GetVertErrorBand((*it))->GetNHists() << " hists and UseSpreadError = " 
                << htst->GetVertErrorBand((*it))->GetUseSpreadError() << std::endl;

   }

   TMatrixD errMatrixStat = htst->GetStatErrorMatrix();
   //
   int NR = errMatrixStat.GetNrows();
   int NC = errMatrixStat.GetNcols();
   std::cout << " Stat Error Matris " << std::endl;   
   for ( int nr=0; nr<NR; ++nr )
   {
      for ( int nc=0; nc<NC; ++nc )
      {
         std::cout << errMatrixStat[nr][nc] << " " ;
      }
      std::cout << std::endl;
   }
   std::cout << " -------------- " << std::endl;
   
   std::vector<std::string> names = htst->GetSysErrorMatricesNames();   
   for (std::vector<std::string>::const_iterator itName = names.begin() ; itName != names.end() ; ++itName)
   {
      TMatrixD errMatrixSys = htst->GetSysErrorMatrix(*itName, false, false);
      //
      NR = errMatrixSys.GetNrows();
      NC = errMatrixSys.GetNcols(); 
      std::cout << " Sys Error Matrix ---> " << *itName << std::endl;  
      for ( int nr=0; nr<NR; ++nr )
      {
         for ( int nc=0; nc<NC; ++nc )
         {
            std::cout << errMatrixSys[nr][nc] << " " ;
         }
         std::cout << std::endl;
      }
      std::cout << " -------------- " << std::endl;
   }
   
   TMatrixD errMatrixTot  = htst->GetTotalErrorMatrix(true); // incl. stat err, but not as "frac" (shape) neither norm.cov.
   //
   NR = errMatrixTot.GetNrows();
   NC = errMatrixTot.GetNcols();   
   for ( int nr=0; nr<NR; ++nr )
   {
      for ( int nc=0; nc<NC; ++nc )
      {
         std::cout << errMatrixTot[nr][nc] << " " ;
      }
      std::cout << std::endl;
   }   
   std::cout << " ----------------- " << std::endl;

   TH1D totErr = htst->GetTotalError(true); //total error.  sys + stat
   TH1D totSysErr = htst->GetTotalError(false); //total sys error.  no stat.
   for( int i = 1; i <= htst->GetNbinsX(); ++i )
   {
      std::cout << "bin " << i << std::endl;
      std::cout << "     val = " << htst->GetBinContent(i) << std::endl;
      std::cout << "     stat = " << htst->GetBinError(i) << std::endl;
      std::cout << "     totErr = " << totErr.GetBinContent(i) << std::endl;
      std::cout << "     totSysErr = " << totSysErr.GetBinContent(i) << std::endl;
   }

   return;

}
