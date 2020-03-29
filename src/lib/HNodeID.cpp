#include <sys/ioctl.h>
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include <iostream>
#include <regex>

#include <Poco/UUIDGenerator.h>
#include <Poco/Checksum.h>

#include "HNodeID.h"

HNodeID::HNodeID()
{
    // Initilize the ObjID value
    memset( data, 0, sizeof( data ) );

    valid = false;
}

HNodeID::HNodeID( HNodeID &src )
{
    memcpy( data, src.data, sizeof( data ) );
    valid = src.valid;
}

HNodeID::~HNodeID()
{

}

void
HNodeID::clear()
{
    memset( data, 0, sizeof( data ) );
    valid = false;
}

bool
HNodeID::isValid()
{
    return valid;
}

void
HNodeID::create()
{
    Poco::UUIDGenerator generator;
    Poco::UUID uuid = generator.createOne();
    uuid.copyTo( (char *) data );
    valid = true;
}

void 
HNodeID::setValue( uint8_t *valBuf )
{
    memcpy( (uint8_t *)data, valBuf, sizeof data );
    valid = true;
}

bool
HNodeID::setFromStr( std::string value )
{
    // Tokenize on the ':' characters
    const std::regex sepre(":"); 

    // Clear existing data
    clear();

    // Break the provided string into tokens
    std::sregex_token_iterator valIter( value.begin(), value.end(), sepre, -1 );
 
    // Walk the tokens converting to binary representation.
    uint cnt = 0;
    while( valIter != std::sregex_token_iterator() )
    {
        uint8_t byte;

        std::string tokStr = *valIter;

        // Error if an element is longer than 2 characters
        if( tokStr.size() != 2 )
            return true;

        // Try the conversion, error if it is unsuccessful
        if( sscanf( tokStr.c_str(), "%hhx", &byte ) != 1 )
            return true;

        // Add it to the data array
        data[ cnt ] = byte;

        // Keep running total of elements
        cnt += 1;

        // Next token
        valIter++;
    }

    // Make sure the right number of elements where processed.
    if( cnt != sizeof data )
        return true;

    // Success
    valid = true;
    return false;    
}

uint8_t* 
HNodeID::getPtr()
{
    if( valid == false )
        return NULL;
    
    return (uint8_t *) data; 
}

bool
HNodeID::getData( uint8_t *bufPtr )
{
    memset( bufPtr, 0, sizeof data );

    if( valid )
        memcpy( bufPtr, (uint8_t *) data, sizeof data );

    return valid; 
}

bool
HNodeID::getStr( std::string &value )
{
    value.clear();

    if( valid == true )
    {
        char tmpStr[64];
        uint8_t *tmpPtr = (uint8_t *) &data;

        sprintf( tmpStr, "%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx",
                            tmpPtr[0], tmpPtr[1], tmpPtr[2], tmpPtr[3], tmpPtr[4], tmpPtr[5], tmpPtr[6], tmpPtr[7],
                            tmpPtr[8], tmpPtr[9], tmpPtr[10], tmpPtr[11], tmpPtr[12], tmpPtr[13], tmpPtr[14], tmpPtr[15]);

        value = tmpStr;
    }

    return valid;
}

bool
HNodeID::operator==( HNodeID &cmp )
{
    // check if our data is valid
    if( (valid == false) || (cmp.valid == false) )
        return false;

    // Check for match
    if( memcmp( (uint8_t *)data, (uint8_t *)cmp.data, sizeof data ) == 0 )
        return true;

    // Match Failed
    return false;
}

uint32_t 
HNodeID::getCRC32()
{
    Poco::Checksum sum;
    sum.update( (const char *) data, sizeof data );
    return sum.checksum();
}

std::string 
HNodeID::getCRC32AsHexStr()
{
    std::string result;
    char buf[16];
    uint32_t sum = getCRC32();
    sprintf( buf, "%8.8x", sum );
    result = buf;
    return result;
}


