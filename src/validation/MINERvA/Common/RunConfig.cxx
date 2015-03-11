#include "validation/MINERvA/Common/RunConfig.h"

#include "Messenger/Messenger.h"
#include "Utils/StringUtils.h"

#include "validation/MINERvA/Common/ExpData.h"

#include "libxml/parser.h"
#include "libxml/xmlmemory.h"
// #include "libxml/xmlreader.h" // moved to RunConfig.h

using namespace genie;
using namespace genie::utils;

RunConfig::RunConfig( int argc, char ** argv )
{

  fLineParser = new CmdLnArgParser( argc, argv );
  fGSimFiles  = new GSimFiles( false, 10 ); // no chain; max 10 models/versions
  fExpData    = new ExpData();

  if( fLineParser->OptionExists('g') ) 
  {
     string inputs = fLineParser->ArgAsString('g');
     bool ok = fGSimFiles->LoadFromFile(inputs);
     if(!ok) { 
        LOG("gvldtest", pFATAL) 
          << "Could not read validation program inputs from: " << inputs;
        exit(1);
     }
  }
  else
  {
     LOG("gvldtest", pFATAL)
        << "NO MC(GENIE) sample is provided\n Useage format: ./gvld_MINERvA_Q2_QE_CC -g <mc-spec>.xml -d <dataset-spec>.xml \n exit";
     exit(1);
  }
    
  if ( fLineParser->OptionExists('d') )
  {
     string dsets = fLineParser->ArgAsString('d');
     bool ok = LoadExpData( dsets );
     if(!ok) 
     { 
        LOG("gvldtest", pFATAL) 
          << "Could not read validation program datasets from: " << dsets;
        exit(1);
     }
  }
  else
  {
     LOG("gvldtest", pFATAL)
        << "NO validation dataset is provided\n Useage format: ./gvld_MINERvA_Q2_QE_CC -g <mc-spec>.xml -d <dataset-spec>.xml \n exit";
     exit(1);
  }
  
}

RunConfig::~RunConfig()
{

   delete fLineParser;
   delete fGSimFiles;
   delete fExpData;

}

// NOTE: just in case...
//
//Enum xmlReaderTypes {
//    XML_READER_TYPE_NONE = 0
//    XML_READER_TYPE_ELEMENT = 1
//    XML_READER_TYPE_ATTRIBUTE = 2
//    XML_READER_TYPE_TEXT = 3
//    XML_READER_TYPE_CDATA = 4
//    XML_READER_TYPE_ENTITY_REFERENCE = 5
//    XML_READER_TYPE_ENTITY = 6
//    XML_READER_TYPE_PROCESSING_INSTRUCTION = 7
//    XML_READER_TYPE_COMMENT = 8
//    XML_READER_TYPE_DOCUMENT = 9
//    XML_READER_TYPE_DOCUMENT_TYPE = 10
//    XML_READER_TYPE_DOCUMENT_FRAGMENT = 11
//    XML_READER_TYPE_NOTATION = 12
//    XML_READER_TYPE_WHITESPACE = 13
//    XML_READER_TYPE_SIGNIFICANT_WHITESPACE = 14
//    XML_READER_TYPE_END_ELEMENT = 15
//    XML_READER_TYPE_END_ENTITY = 16
//    XML_READER_TYPE_XML_DECLARATION = 17
//}

bool RunConfig::LoadExpData( const std::string& dset )
{

   xmlTextReader* reader = 0;
   reader = xmlNewTextReaderFilename(dset.c_str());
   if(reader == NULL) 
   {
      xmlFree(reader);
      return false;
   }

   int ret = 0;

   while ( reader != NULL )
   {

      ret = xmlTextReaderRead(reader);
      
      while ( ret==1 )
      {

         xmlChar*   name  = xmlTextReaderName     (reader);
         int        type  = xmlTextReaderNodeType (reader);

	 // end of exp.data input
	 //
	 if ( xmlStrEqual(name,(const xmlChar*)"exp_data") && type==XML_READER_TYPE_END_ELEMENT )
	 {
	    xmlFree(name);
	    xmlFreeTextReader(reader);
	    return true;
	 }
	 
	 // cycle till founnd start of the record for another dataset
	 //
	 while ( ret==1 && !( xmlStrEqual(name,(const xmlChar*)"dataset") && type==XML_READER_TYPE_ELEMENT ) )
	 {
	    ret = xmlTextReaderRead(reader);
            name  = xmlTextReaderName     (reader);
            type  = xmlTextReaderNodeType (reader);
	    //
	    // end of exp.data input (double check/protection)
	    //
	    if ( xmlStrEqual(name,(const xmlChar*)"exp_data") && type==XML_READER_TYPE_END_ELEMENT )
	    {
	       xmlFree(name);
	       xmlFreeTextReader(reader);
	       return true;
	    }
	 }
	 
	 // found dataset record spec
	 
	 bool code = ProcessDatasetRecord( reader );
	 
         if ( !code || ret!=1 )
         {
            xmlFree(name);
	    xmlFreeTextReader(reader);
	    if ( code )
	    {
	       return true; // normal case, just EOF
	    }
	    else
	    {
	       return false; // something isn't quite right...
	    }
         }

	 ret = xmlTextReaderRead(reader);
	 xmlFree(name);
      
      } // end loop over ret=1

      xmlFreeTextReader(reader);

   } // end loop on reader!=0
   
   return true;

}

bool RunConfig::ProcessDatasetRecord( xmlTextReader* reader )
{

// NOTE: Do NOT free the xml reader here !!!

// OK, we get here is name=="dataset"
// Do we need to re-check on that ?
//
   xmlChar*   name  = xmlTextReaderName     (reader);
   int        type  = xmlTextReaderNodeType (reader);
//   int        depth = xmlTextReaderDepth    (reader); 
   //  
   // NOTE: xmlStrcmp is the opposite to xmlStrEqual
   //
   if ( (!xmlStrcmp(name, (const xmlChar*)"dataset")) && type==XML_READER_TYPE_ELEMENT )
   {
      // something strange !!!
      // got here by mistake - give up
      // return false
   }

   int ret = xmlTextReaderRead(reader);
   
   // loop over the dataset record
   // break the loop if at teh end of the dataset record
   //
   while ( ret == 1 ) // keep reading til the end of interaction
   {

      name = xmlTextReaderName     (reader);
      type = xmlTextReaderNodeType (reader);

      if ( xmlStrEqual(name,(const xmlChar*)"dataset") && type==XML_READER_TYPE_END_ELEMENT ) 
      {
//         std::cout << " Found end of dataset-block; breaking the loop" << std::endl;
	 break;
      }
      
      if ( xmlStrEqual(name, (const xmlChar*)"location") && type==XML_READER_TYPE_ELEMENT )
      {
         std::string path;
	 path.append( (const char*)xmlTextReaderGetAttribute(reader,(const xmlChar*)"path") );
	 fExpData->ReadExpData( path );
      }
      
      ret = xmlTextReaderRead(reader);
      xmlFree(name);
         
   } // loop over valid read-in (ret==1)

   xmlFree(name);

   return true;

}

bool RunConfig::LoadMC( const std::string& mcset )
{

   return true;

}
