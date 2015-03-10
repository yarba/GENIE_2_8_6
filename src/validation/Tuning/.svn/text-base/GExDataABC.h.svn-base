//____________________________________________________________________________
/*!

\class    genie::tune::GExDataABC

\brief    Pure abstract base class (pABC) for experimental data plugged into
          the validation and tuning frameworks.

\author   Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
          University of Liverpool & STFC Rutherford Appleton Laboratory

          Adapted from VALOR oscillation fitting group code developed by
          C. Andreopoulos (Liverpool & STFC RAL), T. Dealtry (Oxford),            
          S. Dennis (Warwick), L. Escudero (IFIC), N.Grant (Lancaster) and
          D.Sgalaberna (ETHZ).

\created  Feb 05, 2015

\cpright  Copyright (c) 2003-2015, GENIE Neutrino MC Generator Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
          or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#ifndef _genie_expt_data_abc_h_
#define _genie_expt_data_abc_h_

#include <string>
#include <TROOT.h>

class TObjArray;

using std::string;

namespace genie {
namespace tune {

class GExModelABC;
class GFitParamList;

class GExDataABC
{
public:
  virtual ~GExDataABC();

  // Identify fit data with a name (to be propagated in output files, etc)
  virtual string Name(void) = 0;  

  // Read data from the input file (ROOT, XML, other)
  // Use input option as needed to specify what exactly to read from the input file.
  // Return true if data were succesfully read out
  virtual bool Read(string file, Option_t * opt = 0) = 0;  

  // Generate mock data (toy-MC)
  // Toy-MC is generated from using the input model 
  virtual bool Generate (GExModelABC * model, const GFitParamList & fp, bool fluctuate=true) = 0;

  // Write datasets to the input container or to the input file
  // The input tag will be appended to the return value of Name() and the name of each stored object.
  // For example, if Name() returns "t2k_cc1pi_2014" and tag is "nominal", an object named "pi_spectrum" will be
  // saved out "t2k_cc1pi_2014_pi_spectrum_nominal".
  // The input write_opt option can be used to specify what exactly to write out. For example, if data are held
  // in arrays (as primary source, for CPU efficiency) and also in histograms (for their graphical representation)
  // one can choose to write out one or both. Depending on how each concrete implementation works and what
  // is needed by each analysis step, this may save time and/or space.
  // If null, everything should be written out.
  // The input file_opt option can be used to write out a number of results into the same file (e.g. by calling
  // the Write() method with file_opt="recreate" in the first instance and with file_opt="update" subsequently).
  // If null, the default is "recreate".
  virtual bool Write (TObjArray * collection, const char * tag = "data", Option_t * write_opt = 0) = 0;
  virtual bool Write (const char * filename,  const char * tag = "data", Option_t * write_opt = 0, Option_t * file_opt = 0) = 0;

  // Number of bins in fit data, or number of events for unbinned fits
  virtual int NumOfBins (void) = 0;

  // Calculate the specified statistic
  // The corresponding prediction is taken from the input model.
  // Similar considerations as in Write() method, see above, apply for how this prediction is generated.
  // This method calculates the statistical term. 
  // The penalty terms is requested from the input model, which knows of all the systematic tweaks applied to it
  virtual double Statistic (GExModelABC * pred, const GFitParamList & fp,   Option_t * stat_opt = "likelihood-ratio") = 0;
  virtual double Statistic (GExModelABC * pred, Option_t * stat_opt = "likelihood-ratio") = 0;

protected:
   GExDataABC();
};

} // tune
} // genie

#endif

