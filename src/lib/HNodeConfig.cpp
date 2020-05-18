#include <iostream>
#include <iomanip>
#include <fstream>
#include <ios>
#include <exception>

#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "HNodeConfig.h"

namespace pjs = Poco::JSON;
namespace pdy = Poco::Dynamic;

// std::map< std::string, std::string > pairMap;
HNCObj::HNCObj()
{
}

HNCObj::~HNCObj()
{
}

void
HNCObj::clear()
{
    pairMap.clear();
}

HNC_RESULT_T 
HNCObj::updateValue( std::string key, std::string value )
{
    pairMap[ key ] = value;
}

void 
HNCObj::getValuePairs( std::map< std::string, std::string > &pairs )
{
    pairs.clear();

    for( std::map< std::string, std::string >::iterator it = pairMap.begin(); it != pairMap.end(); it++ )
    {
        pairs.insert( *it );
    }    
}

void 
HNCObj::debugPrint( uint offset )
{
    for( std::map< std::string, std::string >::iterator it = pairMap.begin(); it != pairMap.end(); it++ )
    {
        std::cout << std::setw( offset ) << " " << "Value - key: " << it->first << "  value: " << it->second << std::endl;      
    }
}

// std::list< HNCObj > objList;
HNCObjList::HNCObjList( std::string nmval )
{
    name = nmval;
}

HNCObjList::~HNCObjList()
{

}

void
HNCObjList::clear()
{
    for( std::vector< HNCObj >::iterator it = objList.begin(); it != objList.end(); it++ )
    {
        it->clear();
    }

    objList.clear();
}

uint 
HNCObjList::size()
{
    return objList.size();
}
        
HNC_RESULT_T 
HNCObjList::getObjPtr( uint index, HNCObj **objPtr )
{
    *objPtr = NULL;

    if( index >= objList.size() )
        return HNC_RESULT_FAILURE;

    *objPtr = &(objList[index]);

    return HNC_RESULT_SUCCESS;
}

HNC_RESULT_T 
HNCObjList::appendObj( HNCObj **objPtr )
{
    HNCObj newObj;

    *objPtr = NULL;

    objList.push_back( newObj );

    *objPtr = &objList[ (objList.size() - 1) ];
    
    return HNC_RESULT_SUCCESS;
}

void 
HNCObjList::debugPrint( uint offset )
{
    uint index = 0;
    for( std::vector< HNCObj >::iterator it = objList.begin(); it != objList.end(); it++ )
    {
        std::cout << std::setw( offset ) << " " << "Array Object #" << index << std::endl;
        it->debugPrint( offset + 2 );
        index += 1;
    }
}

// std::string name;
// std::map< std::string, std::string > pairMap;
// std::map< std::string, HNCObjList > listMap;
HNCSection::HNCSection( std::string nmValue )
{
    name = nmValue;
}

HNCSection::~HNCSection()
{

}

void
HNCSection::clear()
{
    name.clear();
    pairMap.clear();

    for( std::map< std::string, HNCObjList >::iterator it = listMap.begin(); it != listMap.end(); it++ )
    {
        it->second.clear();
    }

    listMap.clear();
}

std::string 
HNCSection::getName()
{
    return name;
}

HNC_RESULT_T 
HNCSection::updateValue( std::string key, std::string value )
{
    pairMap[ key ] = value;
}

HNC_RESULT_T 
HNCSection::clearValue( std::string key )
{
    pairMap.erase( key );
}

HNC_RESULT_T 
HNCSection::getValueByName( std::string key, std::string &value )
{
    value.clear();

    std::map< std::string, std::string >::iterator it = pairMap.find( key );

    if( it == pairMap.end() )
        return HNC_RESULT_FAILURE;

    value = it->second;

    return HNC_RESULT_SUCCESS;
}

void 
HNCSection::getValuePairs( std::map< std::string, std::string > &pairs )
{
    pairs.clear();

    for( std::map< std::string, std::string >::iterator it = pairMap.begin(); it != pairMap.end(); it++ )
    {
        pairs.insert( *it );
    }
}

void 
HNCSection::getListPairs( std::map< std::string, HNCObjList* > &pairs )
{
    pairs.clear();

    for( std::map< std::string, HNCObjList >::iterator it = listMap.begin(); it != listMap.end(); it++ )
    {
        pairs.insert( std::pair< std::string, HNCObjList* >( it->first, &(it->second) ) );
    }
}

HNC_RESULT_T 
HNCSection::updateList( std::string name, HNCObjList **listPtr )
{
    *listPtr = NULL;

    std::map< std::string, HNCObjList >::iterator it = listMap.find( name );

    if( it == listMap.end() )
    {
        HNCObjList newList( name );
        listMap.insert( std::pair< std::string, HNCObjList >( name, newList ) );
        it = listMap.find( name );
    }

    *listPtr = &(it->second);

    return HNC_RESULT_SUCCESS;
}

HNC_RESULT_T 
HNCSection::clearList( std::string name )
{
    std::map< std::string, HNCObjList >::iterator it = listMap.find( name );

    if( it == listMap.end() )
    {
        return HNC_RESULT_SUCCESS;
    }

    it->second.clear();

    listMap.erase( it );

    return HNC_RESULT_SUCCESS;
}

void 
HNCSection::debugPrint( uint offset )
{
    std::cout << std::setw( offset ) << " " << "=== Section: " << name << " ===" << std::endl;

    for( std::map< std::string, std::string >::iterator it = pairMap.begin(); it != pairMap.end(); it++ )
    {
        std::cout << std::setw( offset+2 ) << " " << "Value - key: " << it->first << "  value: " << it->second << std::endl;      
    }

    for( std::map< std::string, HNCObjList >::iterator it = listMap.begin(); it != listMap.end(); it++ )
    {
        std::cout << std::setw( offset+2 ) << " " << "Object Array - key: " << it->first << std::endl;              
        it->second.debugPrint( offset + 4 );
    }
}

// std::string baseName;
// std::string instanceName;
// std::map< std::string, HNCSection > sectionMap;
HNodeConfig::HNodeConfig()
{

}

HNodeConfig::~HNodeConfig()
{

}

void 
HNodeConfig::clear()
{
    for( std::map< std::string, HNCSection >::iterator it = sectionMap.begin(); it != sectionMap.end(); it++ )
    {
        it->second.clear();
    }

    sectionMap.clear();
}

HNC_RESULT_T 
HNodeConfig::updateSection( std::string name, HNCSection **secPtr )
{
    *secPtr = NULL;

    std::map< std::string, HNCSection >::iterator it = sectionMap.find( name );

    if( it == sectionMap.end() )
    {
        HNCSection newSec( name );
        sectionMap.insert( std::pair< std::string, HNCSection >( name, newSec ) );
        it = sectionMap.find( name );
    }

    *secPtr = &(it->second);

    return HNC_RESULT_SUCCESS;
}

void 
HNodeConfig::getSectionPtrs( std::vector< HNCSection* > &sectionList )
{
    sectionList.clear();

    for( std::map< std::string, HNCSection >::iterator it = sectionMap.begin(); it != sectionMap.end(); it++ )
    {
        sectionList.push_back( &(it->second) );
    }    
}

HNC_RESULT_T 
HNodeConfig::removeSection( std::string name )
{
    std::map< std::string, HNCSection >::iterator it = sectionMap.find( name );

    if( it != sectionMap.end() )
    {
        it->second.clear();
        sectionMap.erase( it ); 
        return HNC_RESULT_SUCCESS;
    }

    return HNC_RESULT_FAILURE;
}

void 
HNodeConfig::debugPrint( uint offset )
{
    std::cout << std::setw( offset ) << " " << "HNODE CONFIGURATION FILE" << std::endl;

    for( std::map< std::string, HNCSection >::iterator it = sectionMap.begin(); it != sectionMap.end(); it++ )
    {
        it->second.debugPrint( offset + 2 );
    }    
}

// std::string rootPath;
HNodeConfigFile::HNodeConfigFile()
{
    rootPath = HNCFG_DEFAULT_CFG_PATH;
}

HNodeConfigFile::~HNodeConfigFile()
{
}

void 
HNodeConfigFile::setRootPath( std::string value )
{
    rootPath = value;
}

std::string 
HNodeConfigFile::getRootPath()
{
    return rootPath;
}

HNC_RESULT_T 
HNodeConfigFile::verifyFilename( std::string baseName, std::string instanceName, std::string &fbase )
{
    char fname[ 256 ];
 
    fbase.clear();

    if( baseName.empty() == true )
        return HNC_RESULT_FAILURE;

    if( instanceName.empty() == true )
        return HNC_RESULT_FAILURE;
  
    sprintf( fname, "hncfg_%s_%s", baseName.c_str(), instanceName.c_str() );

    fbase = fname;

    return HNC_RESULT_SUCCESS;
}

bool 
HNodeConfigFile::fileExists( std::string fPath )
{
    std::string fname;
 
    // Then check for the file itself
    Poco::Path path( fPath );
    Poco::File file( path );
    if( file.exists() == false )
        return false;

    // And finally ensure that it is a normal file.
    if( file.isFile() == false )
        return false;

    // Existence determined
    return true;
}

HNC_RESULT_T
HNodeConfigFile::createDirectories()
{
    Poco::Path path( rootPath );
    path.makeDirectory();

    Poco::File dir( path );

    if( dir.exists() )
    {
        if( dir.isDirectory() )
            return HNC_RESULT_SUCCESS;
        else
            return HNC_RESULT_FAILURE;
    }

    // Create any intermediate directories
    try
    {
        dir.createDirectories();
    }
    catch( Poco::Exception ex )
    {
        return HNC_RESULT_FAILURE;
    }

    return HNC_RESULT_SUCCESS;
}

HNC_RESULT_T 
HNodeConfigFile::createFile( std::string fPath )
{
    if( fileExists( fPath ) == true )
    {
        return HNC_RESULT_SUCCESS;
    }

    // Get it as a Poco path
    Poco::Path path( fPath );
    Poco::File file( path );
 
    // Create the file
    try
    {
        file.createFile();
    }
    catch( Poco::Exception ex )
    {
        return HNC_RESULT_FAILURE;
    }

    // Success
    return HNC_RESULT_SUCCESS;
}

HNC_RESULT_T 
HNodeConfigFile::moveFile( std::string srcFN, std::string dstFN )
{
    // Make sure at least the source exists
    if( fileExists( srcFN ) == false )
        return HNC_RESULT_FAILURE;

    // Source path poco objects
    Poco::Path srcPath( srcFN );
    Poco::File srcFile( srcPath );

    // Move the file
    try
    {
        srcFile.moveTo( dstFN );
    }
    catch( Poco::Exception ex )
    {
        return HNC_RESULT_FAILURE;
    }

    return HNC_RESULT_SUCCESS;
}


bool 
HNodeConfigFile::configExists( std::string baseName, std::string instanceName )
{
    std::string fname;

    // Generate and verify filename
    if( verifyFilename( baseName, instanceName, fname ) != HNC_RESULT_SUCCESS )
    {
        return false;
    }

    // Tack on the json extension
    fname += ".json";    

    Poco::Path path( rootPath );
    path.makeDirectory();
    path.setFileName( fname );

    // Existence determined
    return fileExists( path.toString() );
}

HNC_RESULT_T 
HNodeConfigFile::loadConfig( std::string baseName, std::string instanceName, HNodeConfig &config )
{
    std::string fname;

    // Clear any existing config information
    config.clear();

    // Generate and verify filename
    if( verifyFilename( baseName, instanceName, fname ) != HNC_RESULT_SUCCESS )
    {
        return HNC_RESULT_FAILURE;
    }

    // Tack on the json extension
    fname += ".json";    

    // Build target file path
    Poco::Path path( rootPath );
    path.makeDirectory();
    path.setFileName( fname );
    
    // Check on the file
    Poco::File file( path );

    if( file.exists() == false || file.isFile() == false )
    {
        return HNC_RESULT_FAILURE;
    }

    // Open a stream for reading
    std::ifstream its;
    its.open( path.toString() );

    if( its.is_open() == false )
    {
        return HNC_RESULT_FAILURE;
    }

    // Invoke the json parser
    try
    {
        // Attempt to parse the json    
        pjs::Parser parser;
        pdy::Var varRoot = parser.parse( its );
        its.close();

        // Get a pointer to the root object
        pjs::Object::Ptr jsRoot = varRoot.extract< pjs::Object::Ptr >();

        // Iterate through the fields, make into sections.
        for( pjs::Object::ConstIterator it = jsRoot->begin(); it != jsRoot->end(); it++ )
        {
            if( jsRoot->isObject( it ) == true )
            {
                HNCSection *secPtr;
                config.updateSection( it->first, &secPtr );

                pjs::Object::Ptr jsSec = it->second.extract< pjs::Object::Ptr >();

                for( pjs::Object::ConstIterator sit = jsSec->begin(); sit != jsSec->end(); sit++ )
                {

                    if( sit->second.isString() == true )
                    {    
                        secPtr->updateValue( sit->first, sit->second.extract< std::string >() ); 
                    }
                    else if( jsSec->isArray( sit ) == true )
                    {
                        HNCObjList *listPtr;
                        secPtr->updateList( sit->first, &listPtr );

                        pjs::Array::Ptr jsArr = jsSec->getArray( sit->first );
                  
                        uint index = 0;
                        for( uint index = 0; index < jsArr->size(); index++ )
                        {
                            if( jsArr->isObject( index ) == true )
                            {
                                pjs::Object::Ptr jsAObj = jsArr->getObject( index );

                                HNCObj *aobjPtr;
                                listPtr->appendObj( &aobjPtr );

                                for( pjs::Object::ConstIterator aoit = jsAObj->begin(); aoit != jsAObj->end(); aoit++ )
                                {
                                    if( aoit->second.isString() == true )
                                    {    
                                        aobjPtr->updateValue( aoit->first, aoit->second.extract< std::string >() ); 
                                    }
                                }
                            }
                            else
                            {
                                // Unrecognized config element at array layer
                            }
                        }
                    }
                    else
                    {
                        // Unrecognized config element at section layer.  Log? Error?
                    }
                }
            }
            else
            {
                // Unrecognized config element at root layer.  Log? Error?
            }

        }

    }
    catch( Poco::Exception ex )
    {
        return HNC_RESULT_FAILURE;
    }
    

#if 0
    std::string json = "{ \"test\" : { \"property\" : \"value\" } }";
    pjs::Parser parser;
    pdy::Var result = parser.parse( json );

    // use pointers to avoid copying
    pjs::Object::Ptr object = result.extract< pjs::Object::Ptr >();
    pdy::Var test = object->get( "test" ); // holds { "property" : "value" }
    pjs::Object::Ptr subObject = test.extract< pjs::Object::Ptr >();
    test = subObject->get( "property" );
    std::string val = test.toString(); // val holds "value"

    std::cout << "JSON value: " << val << std::endl;

    pjs::Stringifier::stringify( result, std::cout, 1 );
#endif

}

HNC_RESULT_T 
HNodeConfigFile::saveConfig( std::string baseName, std::string instanceName, HNodeConfig config )
{
    std::string fname;

    // Create directories if necessary
    if( createDirectories() != HNC_RESULT_SUCCESS )
    {
        return HNC_RESULT_FAILURE;
    }

    // Generate and verify filename
    if( verifyFilename( baseName, instanceName, fname ) != HNC_RESULT_SUCCESS )
    {
        return HNC_RESULT_FAILURE;
    }

    // Tack on the json extension
    fname += ".json";    

    // Build target file path
    Poco::Path tpath( rootPath );
    tpath.makeDirectory();
    tpath.setFileName( fname );

    // Create the target file if it doesn't exist
    if( createFile( tpath.toString() ) != HNC_RESULT_SUCCESS )
    {
        return HNC_RESULT_FAILURE;
    }

    // Build temporary file path
    fname += "_new";
    Poco::Path npath( rootPath );
    npath.makeDirectory();
    npath.setFileName( fname );

    // Create the temporary file
    if( createFile( npath.toString() ) != HNC_RESULT_SUCCESS )
    {
        return HNC_RESULT_FAILURE;
    }

    // Open an output stream to the temporary file
    std::ofstream ots;
    ots.open( npath.toString(), std::ios::out | std::ios::trunc );

    if( ots.is_open() == false )
    {
        return HNC_RESULT_FAILURE;
    }

    // Create a json root object
    pjs::Object jsRoot;

    // Get pointers to the sections in the config file
    std::vector< HNCSection* > sectionList;
    config.getSectionPtrs( sectionList );

    // Walk each section to build up json
    for( std::vector< HNCSection* >::iterator it = sectionList.begin(); it != sectionList.end(); it++ )
    {
        // The top level json object for each section
        pjs::Object jsSec;
         
        // Get all of the str-str pairs for the section
        // and add them to the section json object
        std::map< std::string, std::string > values;
        (*it)->getValuePairs( values );
        for( std::map< std::string, std::string >::iterator vit = values.begin(); vit != values.end(); vit++ )
        {
            jsSec.set( vit->first, vit->second );
        }

        // Get all of the str-array pairs for the section
        // and add them to the section json object
        std::map< std::string, HNCObjList* > lists;
        (*it)->getListPairs( lists );
        for( std::map< std::string, HNCObjList* >::iterator lit = lists.begin(); lit != lists.end(); lit++ )
        {
            // The json array object
            pjs::Array jsArr;

            // Walk through each member of the array 
            // and create a json object to represent it.
            for( uint i = 0; i < lit->second->size(); i++ )
            {
                // Array member json object
                pjs::Object jsLObj;

                // Get object ptr by index
                HNCObj *objPtr;
                lit->second->getObjPtr( i, &objPtr );

                // Extract the str-str pairs from the object 
                // and add them to the json representation.
                std::map< std::string, std::string > objVals;
                objPtr->getValuePairs( objVals );
                for( std::map< std::string, std::string >::iterator oit = objVals.begin(); oit != objVals.end(); oit++ )
                {
                    jsLObj.set( oit->first, oit->second );
                }

                // Add the json object as an array member
                jsArr.add( jsLObj );
            }

            // Add the sub-array as a member of the section object
            jsSec.set( lit->first, jsArr );
        }

        // Add the section object as a member of the root object
        jsRoot.set( (*it)->getName(), jsSec );
    }

    try
    {
        // Write out the generated json
        pjs::Stringifier::stringify( jsRoot, ots, 1 );
    }
    catch( ... )
    {
        return HNC_RESULT_FAILURE;
    }

    // Close the stream
    ots.close();

    // Overwrite the original config file 
    // with the new one.
    if( moveFile( npath.toString(), tpath.toString() ) != HNC_RESULT_SUCCESS )
    {
        return HNC_RESULT_FAILURE;
    }

    // Finished
    return HNC_RESULT_SUCCESS;
}

