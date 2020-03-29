CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(hnode2-core)

FIND_PACKAGE(Poco REQUIRED Foundation Net NetSSL JSON)

SET( HNODE2_LIB_SRC ${CMAKE_HOME_DIRECTORY}/src/lib/HNHttpServer.cpp 
                    ${CMAKE_HOME_DIRECTORY}/src/lib/HNRootHandler.cpp
                    ${CMAKE_HOME_DIRECTORY}/src/lib/HNAvahi.cpp
                    ${CMAKE_HOME_DIRECTORY}/src/lib/HNodeID.cpp
                    ${CMAKE_HOME_DIRECTORY}/src/lib/HNodeConfig.cpp
                    ${CMAKE_HOME_DIRECTORY}/src/lib/HNodeDevice.cpp )

ADD_LIBRARY(hnode2 SHARED ${HNODE2_LIB_SRC})

TARGET_LINK_LIBRARIES(hnode2 PUBLIC
    Poco::Foundation
    Poco::Net
    Poco::NetSSL
    Poco::JSON
)

TARGET_LINK_LIBRARIES(hnode2 PUBLIC
    -lavahi-common
    -lavahi-client
)

ADD_EXECUTABLE(hnode2_test ${CMAKE_HOME_DIRECTORY}/src/test/main.cpp)

TARGET_INCLUDE_DIRECTORIES(hnode2_test PUBLIC ${CMAKE_HOME_DIRECTORY}/src/lib/)
TARGET_LINK_LIBRARIES(hnode2_test PUBLIC hnode2)

