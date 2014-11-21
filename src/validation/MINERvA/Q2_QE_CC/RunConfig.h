#ifndef Val_MINERvA_RunConfig_H
#define Val_MINERvA_RunConfig_H

#include <cstdlib>
#include <cassert>
#include <map>
#include <vector>
#include <string>

// #include "Utils/GSimFiles.h"
#include "Utils/CmdLnArgParser.h"
#include "libxml/xmlreader.h"

class ExpData;

class RunConfig
{

   public:
   
      RunConfig( int argc, char ** argv );
      ~RunConfig();
      const ExpData*       GetExpData()            const { return fExpData; }
//      ExpData*       GetExpData()            const { return fExpData; }
   
   private:
   
     bool LoadExpData( const std::string& );
     bool ProcessDatasetRecord( xmlTextReader* reader );
     bool LoadMC( const std::string& );
   
     genie::CmdLnArgParser* fLineParser;
//     GSimFiles*      fGSimFiles;
     ExpData*               fExpData;

};

#endif
