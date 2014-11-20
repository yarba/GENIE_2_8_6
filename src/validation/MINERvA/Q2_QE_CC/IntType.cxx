#include <TSystem.h>
#include <Riostream.h>

#include "IntType.h"

#include "PDG/PDGCodes.h"
#include "PDG/PDGLibrary.h"
#include "Messenger/Messenger.h"

using namespace genie;

IntType& IntType::operator=( const IntType& type )
{

   fProjectilePDG = type.fProjectilePDG;
   fNucleonPDG    = type.fNucleonPDG;

   return *this;

}

bool IntType::operator==( const IntType& other ) const
{

   if ( fProjectilePDG == other.fProjectilePDG &&
        fNucleonPDG    == other.fNucleonPDG ) return true; 
   
   return false;

}

bool IntType::SetType( const std::string& projname, const std::string& nucname )
{

   TDatabasePDG* table = (PDGLibrary::Instance())->DBase();
   
   TParticlePDG* prj = table->GetParticle( projname.c_str() );   
   if ( !prj )
   {
      LOG("gvldtest", pWARN)           // FIXME : should it be pERROR queue instead of PWARN ???
        << " UNKNOWN PROJECTILE";
      fProjectilePDG=-1;
      fNucleonPDG=-1;
      return false;
   }
   fProjectilePDG = prj->PdgCode();
   
   TParticlePDG* nuc = table->GetParticle( nucname.c_str() );   
   if ( !nuc )
   {
      LOG("gvldtest", pWARN)            // FIXME: should it be pERROR queue instead of PWARN ???
        << " UNKNOWN TARGET NUCLEON";
      fProjectilePDG=-1;
      fNucleonPDG=-1;
      return false;
   }
   fNucleonPDG = nuc->PdgCode();
  
   return true;   

}

const char* IntType::GetProjectileName() const
{

   TDatabasePDG* table = (PDGLibrary::Instance())->DBase();   
   TParticlePDG* prj = table->GetParticle( fProjectilePDG );
   return prj->GetName();   
 
}

const char* IntType::GetTgtNucleonName() const
{

   TDatabasePDG* table = (PDGLibrary::Instance())->DBase();   
   TParticlePDG* tgnuc = table->GetParticle( fNucleonPDG );
   return tgnuc->GetName();   
 
}
