#ifndef __HNODE_ID_H__
#define __HNODE_ID_H__

#include <string>

// A unique 16-byte identifier associated with every HNode entity
class HNodeID
{
    private:
        bool valid;

        uint8_t data[16];

    public:
        HNodeID();
        HNodeID( const HNodeID &src );
       ~HNodeID();

        void create();

        void setValue( uint8_t *valBuf );
        bool setFromStr( std::string value );

        uint8_t* getPtr();
        bool getData( uint8_t *bufPtr );
        bool getStr( std::string &rtnVal );

        bool operator==( HNodeID &cmp );

        void clear();
        bool isValid();

        uint32_t getCRC32();
        std::string getCRC32AsHexStr();

        static std::string convertCRC32ToStr( uint32_t value );
        static uint32_t convertStrToCRC32( std::string svalue );
};

#endif // __HNODE_ID_H__
