#include "RWRunConfig.h"

#include "Messenger/Messenger.h"
#include "Utils/StringUtils.h"

using namespace genie;
using namespace genie::utils;

RWRunConfig::RWRunConfig( int argc, char ** argv )
{

  
  // FIXME !!!
  // Still have to implemet:
  // a) range of events (n1,n2) to re-weight
  // b) random seed
  // c) output file name (if desired)
  // d) verbosity control (???)
  
  fLineParser = new CmdLnArgParser( argc, argv );
  fGSimFiles  = new GSimFiles( false, 10 ); // no chain; max 10 models/versions

   if ( fLineParser->OptionExists('g') )
   {
      std::string inputs = fLineParser->ArgAsString('g');
      bool ok = fGSimFiles->LoadFromFile(inputs);    
      if(!ok) 
      { 
         LOG("gvldtest", pFATAL) 
            << "Could not read validation program inputs from: " << inputs;
         exit(1);
      }
   }
   else
   {
       LOG("gvldtest", pFATAL)
          << "NO MC(GENIE) sample is provided\n Useage format: ./rw_app -g <mc-spec>.xml -s <name-of-param-to-tweak> -t <tw1,tw2,...> \n exit";
       exit(1);
   }
   
   if ( fLineParser->OptionExists('s') )
   {
       fParam2Tweak = fLineParser->ArgAsString('s');       
   }
   else
   {
       LOG("gvldtest", pFATAL)
          << "Systematic parameter to tweak is NOT provided\n Useage format: ./rw_app -g <mc-spec>.xml -s <name-of-param-to-tweak> -t <tw1,tw2,...> \n exit";
       exit(1);
   }
   
   if ( fLineParser->OptionExists('t') )
   {
      fTweaks = fLineParser->ArgAsDoubleTokens('t',",");      
   }
   else
   {
       LOG("gvldtest", pFATAL)
          << "Systematic parameter to tweak is NOT provided\n Useage format: ./rw_app -g <mc-spec>.xml -s <name-of-param-to-tweak> t <tw1,tw2,...> \n exit";
       exit(1);
   }
   
   // from here down, optional input/config arguments
   //
   if ( fLineParser->OptionExists('p') )
   {
      fNu2RW = fLineParser->ArgAsIntTokens('p',",");
      // NOTE: Original app is bailing out if the list of nu's to consider is empty
      //       but I don't thihnk it matters...
      //       because the RW producer will update the default based on whether the
      //       list is empty or not
   }
   
  
}

RWRunConfig::~RWRunConfig()
{

   delete fLineParser;
   delete fGSimFiles;

}

