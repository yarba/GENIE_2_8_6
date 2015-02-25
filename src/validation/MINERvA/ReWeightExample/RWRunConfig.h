#ifndef RWRUNCONFIG_H
#define RWRUNCONFIG_H

#include <vector>
#include <string>

#include "Utils/CmdLnArgParser.h"
#include "Utils/GSimFiles.h"

class RWRunConfig
{

   public:
   
      RWRunConfig( int argc, char ** argv );
      ~RWRunConfig();
      const genie::GSimFiles* GetSimFilesHandler()     const { return fGSimFiles; }
      const std::string GetParam2Tweak()               const { return fParam2Tweak; }
      const std::vector<double> GetTweaks()            const { return fTweaks; } 
      const std::vector<int>    GetListOfNu2ReWeight() const { return fNu2RW; }
   
   private:
      
     genie::CmdLnArgParser* fLineParser;
     genie::GSimFiles*      fGSimFiles;
     std::string            fParam2Tweak;
     std::vector<double>    fTweaks;
     std::vector<int>       fNu2RW;

};

#endif
