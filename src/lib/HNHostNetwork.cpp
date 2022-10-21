#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <linux/if_arp.h>

#include <iostream>

#include <Poco/Net/IPAddress.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>

#include "HNHostNetwork.h"

namespace pns = Poco::Net;

HNHNAddress::HNHNAddress()
{
    m_type = HNHN_ADDRTYPE_NOTSET;
    m_ifaceIndex = -1;
    m_prefixBits = 0;
}

HNHNAddress::~HNHNAddress()
{

}

void
HNHNAddress::clear()
{
    m_type = HNHN_ADDRTYPE_NOTSET;
    m_ifaceIndex = -1;
    m_prefixBits = 0;

    m_address.clear();
}

void 
HNHNAddress::setFromStr( std::string address )
{
    m_address = address;

    // Use Poco to do IPAddress validation, etc.
    try 
    {
        pns::IPAddress trialAddr( address );

        switch( trialAddr.family() )
        {
            case pns::AddressFamily::IPv4:
                if( trialAddr.isLoopback() )
                {
                    m_type = HNHN_ADDRTYPE_LOOPBACK_IPV4;
                    return;
                }
                else if( trialAddr.isMulticast() || trialAddr.isBroadcast() )
                {
                    m_type = HNHN_ADDRTYPE_CAST_IPV4;
                    return;
                }
                else if( trialAddr.isLinkLocal() || trialAddr.isSiteLocal() )
                {
                    m_type = HNHN_ADDRTYPE_IPV4;
                    return;
                }
                else
                {
                    m_type = HNHN_ADDRTYPE_INET_IPV4;
                    return;
                }
            break;

            case pns::AddressFamily::IPv6:
                if( trialAddr.isLoopback() )
                {
                    m_type = HNHN_ADDRTYPE_LOOPBACK_IPV6;
                    return;
                }
                else if( trialAddr.isMulticast() || trialAddr.isBroadcast() )
                {
                    m_type = HNHN_ADDRTYPE_CAST_IPV6;
                    return;
                }
                else if( trialAddr.isLinkLocal() || trialAddr.isSiteLocal() )
                {
                    m_type = HNHN_ADDRTYPE_IPV6;
                    return;
                }
                else
                {
                    m_type = HNHN_ADDRTYPE_INET_IPV6;
                    return;
                }
            break;
        }
    } 
    catch( pns::InvalidAddressException ex )
    {
    }

    // Could not parse the address string
    m_type = HNHN_ADDRTYPE_UNKNOWN;
    return;
}

void
HNHNAddress::setInterfaceIndex( uint value )
{
    m_ifaceIndex = value;
}

void
HNHNAddress::setPrefixLength( uint value )
{
    m_prefixBits = value;
}

void
HNHNAddress::setAsEthernet( uint8_t *addrBuf )
{
    char buffer[64];

    m_type = HNHN_ADDRTYPE_ETHERNET;

    snprintf(buffer, sizeof(buffer), " %02x:%02x:%02x:%02x:%02x:%02x", addrBuf[0], addrBuf[1], addrBuf[2], addrBuf[3], addrBuf[4], addrBuf[5]);

    m_address = buffer;
}

void
HNHNAddress::setAsIPV4( uint8_t *addrBuf )
{
    char buffer[256];

    m_type = HNHN_ADDRTYPE_INET_IPV4;

    inet_ntop( AF_INET, addrBuf, buffer, sizeof(buffer) );

    m_address = buffer;
}

void
HNHNAddress::setAsIPV6( uint8_t *addrBuf )
{
    char buffer[256];

    m_type = HNHN_ADDRTYPE_INET_IPV6;

    inet_ntop( AF_INET6, addrBuf, buffer, sizeof(buffer) );

    m_address = buffer;
}

HNHN_ADDRTYPE_T
HNHNAddress::getType()
{
    return m_type;
}

const char* gHNHNAddressTypeStrings[] =
{
   "not-set",       // HNHN_ADDRTYPE_NOTSET,
   "unknown",       // HNHN_ADDRTYPE_UNKNOWN,
   "ipv4",          // HNHN_ADDRTYPE_IPV4,
   "loopback_ipv4", // HNHN_ADDRTYPE_LOOPBACK_IPV4,
   "cast_ipv4",     // HNHN_ADDRTYPE_CAST_IPV4,
   "inet_ipv4",     // HNHN_ADDRTYPE_INET_IPV4,
   "ipv6",          // HNHN_ADDRTYPE_IPV6,
   "loopback_ipv6", // HNHN_ADDRTYPE_LOOPBACK_IPV6,
   "cast_ipv6",     // HNHN_ADDRTYPE_CAST_IPV6,
   "inet_ipv6",     // HNHN_ADDRTYPE_INET_IPV6,
   "ethernet"       // HNHN_ADDRTYPE_ETHERNET
};

std::string
HNHNAddress::getTypeAsStr()
{
    return gHNHNAddressTypeStrings[ m_type ];
}

uint 
HNHNAddress::getInterfaceIndex()
{
    return m_ifaceIndex;
}

uint 
HNHNAddress::getPrefixLength()
{
    return m_prefixBits;
}

std::string
HNHNAddress::getAddress()
{
    return m_address;
}

std::string
HNHNAddress::getURL( std::string protocol, std::string path )
{
    std::string url;

    return url;
}

void 
HNHNAddress::debugPrint( uint offset )
{
    offset += 2;
    printf( "%*.*stype: %s  address: %s\n", offset, offset, " ", getTypeAsStr().c_str(), m_address.c_str() );
}

HNHNInterface::HNHNInterface()
{
    m_index = -1;
    m_type = HNHN_INFTYPE_NOTSET;

    m_isIPV4Gateway = false;
}

HNHNInterface::~HNHNInterface()
{

}

void
HNHNInterface::setIndex( uint value )
{
    m_index = value;
}

void
HNHNInterface::setName( std::string name )
{
    m_name = name;
}

void
HNHNInterface::setTypeEthernet( uint8_t *addrBuf )
{
    m_type = HNHN_INFTYPE_ETHERNET;

    m_hwAddr.setAsEthernet( addrBuf );
}

void
HNHNInterface::setTypeLoopback( uint8_t *addrBuf )
{
    m_type = HNHN_INFTYPE_LOOPBACK;

    m_hwAddr.setAsEthernet( addrBuf );
}

void
HNHNInterface::addAddress( HNHNAddress &srcAddr )
{
    m_addrList.push_back( srcAddr );
}

void
HNHNInterface::setAsDefaultIPV4Gateway( std::string addrStr )
{
    m_isIPV4Gateway = true;
    m_ipv4GatewayAddr.setFromStr( addrStr );
}

uint
HNHNInterface::getIndex()
{
    return m_index;
}

std::string
HNHNInterface::getName()
{
    return m_name;
}

std::string
HNHNInterface::getTypeAsStr()
{
    switch( m_type )
    {
        case HNHN_INFTYPE_ETHERNET:
            return "ethernet";
        break;

        case HNHN_INFTYPE_LOOPBACK:
            return "loopback";
        break;
    }

    return "unknown";
}

bool
HNHNInterface::hasDefaultGateway()
{
    return m_isIPV4Gateway;
}

std::string
HNHNInterface::getDefaultIPV4AddrStr()
{
    for( std::vector< HNHNAddress >::iterator it = m_addrList.begin(); it != m_addrList.end(); it++ )
    {
        if( it->getType() == HNHN_ADDRTYPE_INET_IPV4 )
            return it->getAddress();
    }

    // Return default loopback if no address
    return "127.0.0.1";
}

void 
HNHNInterface::debugPrint( uint offset )
{
    offset += 2;
    printf( "%*.*sintf: %s  index: %d  type: %s\n", offset, offset, " ", m_name.c_str(), m_index, getTypeAsStr().c_str() );
    m_hwAddr.debugPrint( offset );
    offset += 2;
    if( m_isIPV4Gateway == true )
    {
        printf( "%*.*sEgress interface for IPv4 default gateway at: %s\n", offset, offset, " ", m_ipv4GatewayAddr.getAddress().c_str() );
    }

    for( std::vector< HNHNAddress >::iterator it = m_addrList.begin(); it != m_addrList.end(); it++ )
    {
        it->debugPrint( offset );
    }
}

HNHostNetwork::HNHostNetwork()
{
    m_msgseq = 4;
}

HNHostNetwork::~HNHostNetwork()
{

}

void
HNHostNetwork::refreshData()
{
    m_ifaceMap.clear();

    refreshInterfaces();
    refreshAddresses();
    refreshRoutes();
}

std::string
HNHostNetwork::getDefaultIPV4AddrStr()
{
    // Find the interface with the default gateway and 
    // use the ipv4 address that matches the gateway subnet
    for( std::map< std::string, HNHNInterface >::iterator it = m_ifaceMap.begin(); it != m_ifaceMap.end(); it++ )
    {
        if( it->second.hasDefaultGateway() == false )
            continue;

        return it->second.getDefaultIPV4AddrStr();
    }

    // Otherwise return the loopback address
    return "127.0.0.1";
}

void 
HNHostNetwork::debugPrint()
{
    std::cout << "==== Local Network ====" << std::endl;

    for( std::map< std::string, HNHNInterface >::iterator it = m_ifaceMap.begin(); it != m_ifaceMap.end(); it++ )
    {
        it->second.debugPrint(0);
        std::cout << std::endl;
    }
    
    std::cout << "======================" << std::endl;
}

int
HNHostNetwork::executNetlinkRequest( uint sockDomain, uint reqType, uint8_t *sndBuf, uint sndSize, uint8_t *rspBuf, uint rspMaxSize )
{
    int     sock = -1;
    struct  nlmsghdr *nlmsg;
    struct  timeval tv;
    int len;
    struct iovec iov; // = { buf, sizeof(buf) };
    struct sockaddr_nl sa;
    struct msghdr msg;

    sock = socket( AF_NETLINK, SOCK_RAW, sockDomain );
    if( sock < 0 )
    {
        perror("socket failed");
        return -1;
    }

    // point the header and the msg structure pointers into the buffer 
    nlmsg = (struct nlmsghdr *) sndBuf;

    // Fill in the nlmsg header
    nlmsg->nlmsg_len = sndSize;
    nlmsg->nlmsg_type = reqType; // Get the routes from kernel routing table .
    nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlmsg->nlmsg_seq = m_msgseq++; // Sequence of the message packet.
    nlmsg->nlmsg_pid = getpid(); // PID of process sending the request.

    // 2 Sec Timeout to avoid stall
    tv.tv_sec = 2;
    setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(struct timeval) );

    // send msg
    iov = { nlmsg, nlmsg->nlmsg_len };
    msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;

    len = sendmsg( sock, &msg, 0 );

    if( len < 0 )
    {
        perror("send failed");
        return -1;
    }

    iov = { rspBuf, rspMaxSize };
    msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
    len = recvmsg( sock, &msg, 0 );

    close( sock );

    return len;
}

void
HNHostNetwork::refreshRoutes()
{
    uint8_t sndMsg[ NLMSG_LENGTH( sizeof(struct rtmsg) ) ];
    struct nlmsghdr rspBuf[ 8192/sizeof(struct nlmsghdr) ];
    struct nlmsghdr *nh;
    struct rtmsg *route_entry;
    struct rtattr *route_attribute;

    memset( sndMsg, 0, sizeof(sndMsg) );

    int rcvBytes = executNetlinkRequest( NETLINK_ROUTE, RTM_GETROUTE, sndMsg, sizeof(sndMsg), (uint8_t *) rspBuf, sizeof( rspBuf ) );

    // Parse the routing table entries
    for( nh = (struct nlmsghdr *) rspBuf; NLMSG_OK( nh, rcvBytes ); nh = NLMSG_NEXT( nh, rcvBytes ) )
    {
        // The end of multipart message
        if( nh->nlmsg_type == NLMSG_DONE )
        {
            std::cout << "Done Msg" << std::endl;
            return;
        }

        // Do some error handling
        if( nh->nlmsg_type == NLMSG_ERROR )
        {
            std::cout << "Msg Error" << std::endl;
            continue;
        }

        if( ( nh->nlmsg_flags & NLM_F_MULTI ) == 0 )
        {
            std::cout << "Msg Skip" << std::endl;
            continue;
        }

        // Continue with parsing payload 
        // Get the route data 
        route_entry = (struct rtmsg *) NLMSG_DATA(nh);

        // We are just interested in main routing table
        if( route_entry->rtm_table != RT_TABLE_MAIN )
            continue;

        route_attribute = (struct rtattr *) RTM_RTA(route_entry);
        int route_attribute_len = RTM_PAYLOAD(nh);

        // Loop through all attributes
        bool isGateway = false;
        uint intfIndex = 0;
        char gwAddr[256];
        for ( ; RTA_OK(route_attribute, route_attribute_len); route_attribute = RTA_NEXT(route_attribute, route_attribute_len) )
        {            
            switch(route_attribute->rta_type) 
            {
                case RTA_OIF:
                    intfIndex = *(int *)RTA_DATA(route_attribute);
                break;
                
                case RTA_GATEWAY:
                {
                    isGateway = true;
                    inet_ntop( AF_INET, RTA_DATA(route_attribute), gwAddr, sizeof(gwAddr) );
                }
                break;
                
                default:
                break;
            }
        }

        if( isGateway == true )
        {
            for( std::map< std::string, HNHNInterface >::iterator it = m_ifaceMap.begin(); it != m_ifaceMap.end(); it++ )
            {
                if( it->second.getIndex() == intfIndex )
                {
                    it->second.setAsDefaultIPV4Gateway( gwAddr );
                    break;
                }
            }
        }

    }

}

void
HNHostNetwork::refreshInterfaces()
{
    uint8_t sndMsg[ NLMSG_LENGTH( sizeof(struct rtgenmsg) ) ];
    struct nlmsghdr rspBuf[ 8192/sizeof(struct nlmsghdr) ];
    struct nlmsghdr *nh;
    struct rtgenmsg *genh;
    struct rtmsg *route_entry;
    struct rtattr *route_attribute;

    memset( sndMsg, 0, sizeof(sndMsg) );
    genh = (struct rtgenmsg *)( sndMsg + sizeof( struct nlmsghdr ) );
    genh->rtgen_family = AF_PACKET;

    int rcvBytes = executNetlinkRequest( NETLINK_ROUTE, RTM_GETLINK, sndMsg, sizeof(sndMsg), (uint8_t *) rspBuf, sizeof( rspBuf ) );

    // Parse the interface table entries
    for( nh = (struct nlmsghdr *) rspBuf; NLMSG_OK( nh, rcvBytes ); nh = NLMSG_NEXT( nh, rcvBytes ) )
    {
        HNHNInterface iface;

        // Only look at new link stuff
        if( nh->nlmsg_type != RTM_NEWLINK )
        {
            std::cout << "Non-Newlink Skip" << std::endl;
            continue;
        }

        // The end of multipart message
        if( nh->nlmsg_type == NLMSG_DONE )
        {
            std::cout << "Done Msg" << std::endl;
            return;
        }

        // Do some error handling
        if( nh->nlmsg_type == NLMSG_ERROR )
        {
            std::cout << "Msg Error" << std::endl;
            continue;
        }

        if( ( nh->nlmsg_flags & NLM_F_MULTI ) == 0 )
        {
            std::cout << "Msg Skip" << std::endl;
            continue;
        }

        // Continue with parsing payload 
		struct ifinfomsg *ifi_ptr = (struct ifinfomsg *) NLMSG_DATA( nh );
		struct rtattr *attr_ptr = IFLA_RTA( ifi_ptr );
		int attr_len = nh->nlmsg_len - NLMSG_LENGTH( sizeof(*ifi_ptr) );
		
        iface.setIndex( ifi_ptr->ifi_index );

		while( RTA_OK(attr_ptr, attr_len) )
		{     
            switch( attr_ptr->rta_type )
			{
                case IFLA_IFNAME:
                    iface.setName( (char *)RTA_DATA(attr_ptr) );
                break;

			    case IFLA_ADDRESS:
                {
                    if( ifi_ptr->ifi_type == ARPHRD_ETHER )
                        iface.setTypeEthernet( (uint8_t *) RTA_DATA( attr_ptr ) );
                    else if( ifi_ptr->ifi_type == ARPHRD_LOOPBACK )
                        iface.setTypeLoopback( (uint8_t *) RTA_DATA( attr_ptr ) );
                }
                break;
            }

			attr_ptr = RTA_NEXT(attr_ptr, attr_len);
		}

        // Add the interace to the map
        m_ifaceMap.insert( std::pair< std::string, HNHNInterface >( iface.getName(), iface ) );
    }

}

void
HNHostNetwork::refreshAddresses()
{
    uint8_t sndMsg[ NLMSG_LENGTH( sizeof(struct rtgenmsg) ) ];
    struct nlmsghdr rspBuf[ 8192/sizeof(struct nlmsghdr) ];
    struct nlmsghdr *nh;
    struct rtgenmsg *genh;
    struct rtmsg *route_entry;
    struct rtattr *route_attribute;

    memset( sndMsg, 0, sizeof(sndMsg) );
    genh = (struct rtgenmsg *)( sndMsg + sizeof( struct nlmsghdr ) );
    genh->rtgen_family = AF_PACKET;

    int rcvBytes = executNetlinkRequest( NETLINK_ROUTE, RTM_GETADDR, sndMsg, sizeof(sndMsg), (uint8_t *) rspBuf, sizeof( rspBuf ) );

    // Parse the address entries
    HNHNAddress tmpAddr;    
    bool add_address = false;
    for( nh = (struct nlmsghdr *) rspBuf; NLMSG_OK( nh, rcvBytes ); nh = NLMSG_NEXT( nh, rcvBytes ) )
    {
        add_address = false;
        
        tmpAddr.clear();

        // Only look at new link stuff
        if( nh->nlmsg_type != RTM_NEWADDR )
        {
            std::cout << "Non-NewAddr Skip" << std::endl;
            continue;
        }

        // The end of multipart message
        if( nh->nlmsg_type == NLMSG_DONE )
        {
            std::cout << "Done Msg" << std::endl;
            return;
        }

        // Do some error handling
        if( nh->nlmsg_type == NLMSG_ERROR )
        {
            std::cout << "Msg Error" << std::endl;
            continue;
        }

        if( ( nh->nlmsg_flags & NLM_F_MULTI ) == 0 )
        {
            std::cout << "Msg Skip" << std::endl;
            continue;
        }

        // Continue with parsing payload 
		struct ifaddrmsg *ifa_ptr = (struct ifaddrmsg *) NLMSG_DATA( nh );
		struct rtattr *attr_ptr = IFA_RTA( ifa_ptr );
		int attr_len = nh->nlmsg_len - NLMSG_LENGTH( sizeof(*ifa_ptr) );
		
        tmpAddr.setInterfaceIndex( ifa_ptr->ifa_index );
        tmpAddr.setPrefixLength( ifa_ptr->ifa_prefixlen );

		while( RTA_OK(attr_ptr, attr_len) )
		{ 
            switch( attr_ptr->rta_type )
			{
                case IFA_ADDRESS:
                {
                    if( ifa_ptr->ifa_family == AF_INET )
                    {
                        tmpAddr.setAsIPV4( (uint8_t *) RTA_DATA(attr_ptr) );
                        add_address = true;
                    }
                    else if( ifa_ptr->ifa_family == AF_INET6 )
                    {
                        tmpAddr.setAsIPV6( (uint8_t *) RTA_DATA(attr_ptr) );
                        add_address = true;
                    }
                    else
                        printf("Address - family: %d\n", ifa_ptr->ifa_family );
                }
                break;
            }

			attr_ptr = RTA_NEXT(attr_ptr, attr_len);
		}

        if( add_address == true )
            addAddressToInterface( tmpAddr );

    }

}

void
HNHostNetwork::addAddressToInterface( HNHNAddress &srcAddr )
{
    for( std::map< std::string, HNHNInterface >::iterator it = m_ifaceMap.begin(); it != m_ifaceMap.end(); it++ )
    {
        if( it->second.getIndex() == srcAddr.getInterfaceIndex() )
        {
            it->second.addAddress( srcAddr );
            return;
        }
    }
}
