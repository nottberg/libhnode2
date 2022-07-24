CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
PROJECT(libhnode2)

SET( MAJOR_VERSION 2 )
SET( MINOR_VERSION 2 )
SET( PATCH_VERSION 0 )
SET( RELEASE_VERSION 0 )

# Default the install prefix to just /usr
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET( CMAKE_INSTALL_PREFIX /usr CACHE PATH "Default to /usr install" FORCE )
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake) 

SET( VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION} )
SET( DEBVERSION ${VERSION}-${RELEASE_VERSION} )

ADD_DEFINITIONS( -DVERSION=${VERSION} )

FIND_PACKAGE( Poco REQUIRED Util Foundation Net NetSSL JSON )
FIND_PACKAGE( Avahi REQUIRED )

SET( HNODE2_LIB_SRC ${CMAKE_SOURCE_DIR}/src/lib/HNHttpServer.cpp 
                    ${CMAKE_SOURCE_DIR}/src/lib/HNRestHandler.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNReqWaitQueue.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNSigSyncQueue.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNAvahi.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNAvahiBrowser.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNodeID.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNodeConfig.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNEPLoop.cpp
                    #${CMAKE_SOURCE_DIR}/src/lib/HNDeviceRestHandler.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNodeDevice.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNDeviceHealth.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNDeviceLogging.cpp
                    ${CMAKE_SOURCE_DIR}/src/lib/HNDeviceStats.cpp )

SET( HNODE2_LIB_HDR ${CMAKE_SOURCE_DIR}/src/lib/HNHttpServer.h 
                    ${CMAKE_SOURCE_DIR}/src/lib/HNRestHandler.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNReqWaitQueue.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNSigSyncQueue.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNAvahi.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNAvahiBrowser.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNodeID.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNodeConfig.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNEPLoop.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNodeDevice.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNDeviceHealth.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNDeviceLogging.h
                    ${CMAKE_SOURCE_DIR}/src/lib/HNDeviceStats.h )

ADD_LIBRARY( hnode2 SHARED ${HNODE2_LIB_SRC} )
SET_TARGET_PROPERTIES( hnode2 PROPERTIES VERSION ${VERSION} SOVERSION ${MAJOR_VERSION} )

TARGET_LINK_LIBRARIES( hnode2 PUBLIC
    ${Poco_Util_LIBRARY}
    ${Poco_Foundation_LIBRARY}
    ${Poco_Net_LIBRARY}
    ${Poco_NetSSL_LIBRARY}
    ${Poco_JSON_LIBRARY}
)

TARGET_LINK_LIBRARIES( hnode2 PUBLIC
    Avahi::common
    Avahi::client
)

ADD_EXECUTABLE( hnode2_test ${CMAKE_SOURCE_DIR}/src/test/main.cpp )

TARGET_INCLUDE_DIRECTORIES( hnode2_test PUBLIC ${CMAKE_SOURCE_DIR}/src/lib/ )
TARGET_LINK_LIBRARIES( hnode2_test PRIVATE hnode2 )

INSTALL( TARGETS hnode2 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT lib )
INSTALL( TARGETS hnode2_test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT test )

INSTALL( FILES ${HNODE2_LIB_HDR} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/hnode2 COMPONENT dev )

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/dev/hnode2.pc.in ${CMAKE_BINARY_DIR}/hnode2.pc @ONLY )
INSTALL( FILES ${CMAKE_BINARY_DIR}/hnode2.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig COMPONENT dev )

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/dev/debian/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY )

# Generate triggers file
FILE( WRITE "${CMAKE_BINARY_DIR}/triggers" "activate-noawait ldconfig\n")

SET( CPACK_GENERATOR "DEB" )

SET( CPACK_DEB_COMPONENT_INSTALL 1 )
SET( CPACK_COMPONENTS_ALL lib test dev )

SET( CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION} )
SET( CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION} )
SET( CPACK_PACKAGE_VERSION_PATCH ${PATCH_VERSION} )

SET( CPACK_PACKAGE_DESCRIPTION "The libhnode2 library and supporting infrastructure." )
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "The libhnode2 library and supporting infrastructure." )
SET( CPACK_PACKAGE_CONTACT "Curtis Nottberg" )
SET( CPACK_PACKAGE_ARCHITECTURE DEB-DEFAULT )

SET( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )
SET( CPACK_DEBIAN_PACKAGE_SECTION "Libraries" )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Curtis Nottberg" ) #required
SET( CPACK_DEBIAN_PACKAGE_VERSION ${DEBVERSION} )
SET( CPACK_DEBIAN_FILE_NAME DEB-DEFAULT )

SET( CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY ">=" )
SET( CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON )

SET( CPACK_DEBIAN_LIB_PACKAGE_SHLIBDEPS ON )
SET( CPACK_DEBIAN_LIB_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/postinst;${CMAKE_BINARY_DIR}/triggers;" )

INCLUDE(CPack)

