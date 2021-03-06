//____________________________________________________________________________
/*
 Copyright (c) 2003-2013, GENIE Neutrino MC Generator Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Author: Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
         STFC, Rutherford Appleton Laboratory - October 03, 2004

 For the class documentation see the corresponding header file.

 Important revisions after version 2.0.0 :
 @ Nov 29, 2007 - CA
   Was first added in 2.0.1
 @ Feb 09, 2009 - CA
   Moved into the new Coherent package from its previous location  (EVGModules 
   package) 

*/
//____________________________________________________________________________

#include <cstdlib>

#include "Coherent/COHElHadronicSystemGenerator.h"
#include "GHEP/GHepStatus.h"
#include "GHEP/GHepParticle.h"
#include "GHEP/GHepRecord.h"
#include "Messenger/Messenger.h"

using namespace genie;

//___________________________________________________________________________
COHElHadronicSystemGenerator::COHElHadronicSystemGenerator() :
HadronicSystemGenerator("genie::COHElHadronicSystemGenerator")
{

}
//___________________________________________________________________________
COHElHadronicSystemGenerator::COHElHadronicSystemGenerator(string config) :
HadronicSystemGenerator("genie::COHElHadronicSystemGenerator", config)
{

}
//___________________________________________________________________________
COHElHadronicSystemGenerator::~COHElHadronicSystemGenerator()
{

}
//___________________________________________________________________________
void COHElHadronicSystemGenerator::ProcessEventRecord(GHepRecord * evrec) const
{
// This method generates the final state hadronic system in COH elastic events
//
  int imom = evrec->TargetNucleusPosition();
  int pdgc = evrec->Particle(imom)->Pdg();

  TLorentzVector p4nuc = this->Hadronic4pLAB(evrec);
  TLorentzVector v4(0.,0.,0.,0.);

  evrec->AddParticle(pdgc,kIStStableFinalState, imom,-1,-1,-1, p4nuc, v4);
}
//___________________________________________________________________________

