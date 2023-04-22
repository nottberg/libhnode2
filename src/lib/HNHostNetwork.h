#ifndef _HN_LOCAL_NETWORK_H_
#define _HN_LOCAL_NETWORK_H_

#include <string>
#include <map>

typedef enum HNHNAddressTypeEnum
{
    HNHN_ADDRTYPE_NOTSET,
    HNHN_ADDRTYPE_UNKNOWN,
    HNHN_ADDRTYPE_IPV4,
    HNHN_ADDRTYPE_LOOPBACK_IPV4,
    HNHN_ADDRTYPE_CAST_IPV4,
    HNHN_ADDRTYPE_INET_IPV4,
    HNHN_ADDRTYPE_IPV6,
    HNHN_ADDRTYPE_LOOPBACK_IPV6,
    HNHN_ADDRTYPE_CAST_IPV6,
    HNHN_ADDRTYPE_INET_IPV6,
    HNHN_ADDRTYPE_ETHERNET
}HNHN_ADDRTYPE_T;

class HNHNAddress
{    
    public:
        HNHNAddress();
       ~HNHNAddress();

        void clear();

        void setInterfaceIndex( uint value );
        void setPrefixLength( uint value );

        void setAsEthernet( uint8_t *addrBuf );
        void setAsIPV4( uint8_t *addrBuf );
        void setAsIPV6( uint8_t *addrBuf );

        void setFromStr( std::string addrStr );

        HNHN_ADDRTYPE_T  getType();
        std::string getTypeAsStr();

        uint getInterfaceIndex();
        uint getPrefixLength();

        std::string getAddress();
 
        std::string getURL( std::string protocol, std::string path );

        void debugPrint( uint offset );

    private:
        HNHN_ADDRTYPE_T    m_type;

        std::string        m_address;
        uint               m_prefixBits;
        uint               m_ifaceIndex;

};

typedef enum HNHNInterfaceTypeEnumeration
{
    HNHN_INFTYPE_NOTSET,
    HNHN_INFTYPE_ETHERNET,
    HNHN_INFTYPE_LOOPBACK
}HNHN_INFTYPE_T;

class HNHNInterface
{
    public:
        HNHNInterface();
       ~HNHNInterface();

        void setIndex( uint value );
        void setName( std::string name );

        void setTypeEthernet( uint8_t *addrBuf );
        void setTypeLoopback( uint8_t *addrBuf );

        void addAddress( HNHNAddress &srcAddr );

        void setAsDefaultIPV4Gateway( std::string addrStr );

        uint getIndex();
        std::string getName();

        std::string getTypeAsStr();
        
        bool hasDefaultGateway();

        std::string getDefaultIPV4AddrStr();

        void debugPrint( uint offset );

    private:
        HNHN_INFTYPE_T   m_type;
        uint             m_index;
        std::string      m_name;

        HNHNAddress      m_hwAddr;

        bool        m_isIPV4Gateway;
        HNHNAddress m_ipv4GatewayAddr;

        std::vector< HNHNAddress > m_addrList;
};

class HNHostNetwork
{
    public:
        HNHostNetwork();
       ~HNHostNetwork();

        void refreshData();

        std::string getDefaultIPV4AddrStr();

        void debugPrint();

    private:

        void addAddressToInterface( HNHNAddress &srcAddr );

        int executNetlinkRequest( uint sockDomain, uint reqType, uint8_t *sndBuf, uint sndSize, uint8_t *rspBuf, uint rspMaxSize );
        void refreshRoutes();
        void refreshAddresses();
        void refreshInterfaces();

        std::map< std::string, HNHNInterface > m_ifaceMap;

        uint m_msgseq;
};

#endif // _HN_LOCAL_NETWORK_H_