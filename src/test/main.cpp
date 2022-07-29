#include <unistd.h>

#include <sys/epoll.h>

#include <Poco/Util/Application.h>

#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionCallback.h>

#include <Poco/Util/HelpFormatter.h>

#include <iostream>

#include "HNHttpServer.h"
#include "HNAvahi.h"
#include "HNAvahiBrowser.h"
#include "HNodeID.h"
#include "HNodeConfig.h"
#include "HNFormatStrs.h"

#define MAXEVENTS 8

class HNode2TestApp : public Poco::Util::Application
{
    private:
        bool _helpRequested;
        bool _httpServerTest;
        bool _avahiTest;
        bool _avahiBrowserTest;
        bool _hnodeIDTest;
        bool _hnodeConfigTest;
        bool _formatStrsTest;

    protected:
        void initialize( Poco::Util::Application& application )
        {
            this->loadConfiguration();
            Poco::Util::Application::initialize( application );
        }

        void uninitialize()
        {
            Poco::Util::Application::uninitialize();
        }

        void defineOptions( Poco::Util::OptionSet& optionSet )
        {
            _helpRequested    = false;
            _httpServerTest   = false;
            _avahiTest        = false;
            _avahiBrowserTest = false;
            _hnodeIDTest      = false;
            _hnodeConfigTest  = false;
            _formatStrsTest   = false;

            Poco::Util::Application::defineOptions( optionSet );

            optionSet.addOption(
                Poco::Util::Option( "help", "h", "Display usage information." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleHelp ) )
            );

            optionSet.addOption(
                Poco::Util::Option( "httpserver", "t", "Run HNHttpServer test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
            );

            optionSet.addOption(
                Poco::Util::Option( "avahi", "a", "Run Avahi test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
            );

            optionSet.addOption(
                Poco::Util::Option( "avahi-browser", "b", "Run Avahi Browser test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
            );

            optionSet.addOption(
                Poco::Util::Option( "hnodeid", "i", "Run HNodeID test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
            );

            optionSet.addOption(
                Poco::Util::Option( "hnodecfg", "c", "Run HNodeConfig test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
            );

            optionSet.addOption(
                Poco::Util::Option( "formatstr", "f", "Run FormatStr test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
            );

        }

        void displayHelp()
        {
            Poco::Util::HelpFormatter helpFormatter( options() );
            helpFormatter.setCommand( commandName() );
            helpFormatter.setUsage("OPTIONS");
            helpFormatter.setHeader(
                 "A test application for exercising functionality from the HNode2 library."
            );
            helpFormatter.format(std::cout);
        }

        void handleHelp( const std::string& name, const std::string& value )
        {
            _helpRequested = true;
            displayHelp();
            stopOptionsProcessing();
        }

        void handleTest( const std::string& name, const std::string& value )
        {
            std::cout << "Handle option: " << name << std::endl;
            if( name == "httpserver" )
                _httpServerTest = true;
            else if( name == "avahi" )
                _avahiTest = true;
            else if( name == "avahi-browser" )
                _avahiBrowserTest = true;
            else if( name == "hnodeid" )
                _hnodeIDTest = true;
            else if( name == "hnodecfg" )
                _hnodeConfigTest = true;
            else if( name == "formatstr" )
                _formatStrsTest = true;

        }

        int main( const std::vector<std::string> &arguments )
        {
            if( _helpRequested || arguments.size() > 0 )
            {
                displayHelp();
                return 0;
            }

            if( _httpServerTest == true )
            {
                std::cout << "Running HNHttpServer test..." << std::endl;
                //HNHttpServer app;
                //app.start();
                //sleep(300);
            }
            else if( _avahiTest == true )
            {
                std::cout << "Running HNAvahi test..." << std::endl;
                HNAvahi avObj;

                avObj.setID( "_hnode2-http._tcp", "hnode2TestApp" );
                avObj.setPort( 651 );
 
                avObj.setSrvPair( "hnodeid", "12:34:45:67:89:01:23:45:12:34:45:67:89:01:23:45" );
                avObj.setSrvPair( "paired", "false" );
                avObj.setSrvTag( "tst-tag" );

                avObj.start();
                sleep(30);
                avObj.shutdown();
            }
            else if( _avahiBrowserTest == true )
            {
                std::cout << "Running HNAvahiBrowser test..." << std::endl;
                HNAvahiBrowser avObj("_http._tcp");

                // Initialize for event loop
                int epollFD = epoll_create1( 0 );
                if( epollFD == -1 )
                {
                    return Application::EXIT_SOFTWARE;
                }

                // Buffer where events are returned 
                struct epoll_event event;
                struct epoll_event *events = (struct epoll_event *) calloc( MAXEVENTS, sizeof event );

                // Start the Browser Device
                avObj.start();

                // Add the browser event queue to wake us up.
                int eventFD = avObj.getEventQueue().getEventFD();

                event.data.fd = eventFD;
                event.events = EPOLLIN | EPOLLET;
                int s = epoll_ctl( epollFD, EPOLL_CTL_ADD, eventFD, &event );
                if( s == -1 )
                {
                    return Application::EXIT_SOFTWARE;
                }

                // The event loop 
                bool quit = false;
                while( quit == false )
                {
                    int n;
                    int i;

                    // Check for events
                    n = epoll_wait( epollFD, events, MAXEVENTS, 30000 );

                    // EPoll error
                    if( n < 0 )
                    {
                        return Application::EXIT_SOFTWARE;
                    }

                    // Check these critical tasks everytime
                    // the event loop wakes up.
 
                    // If it was a timeout then continue to next loop
                    // skip socket related checks.
                    if( n == 0 )
                    {
                        avObj.shutdown();
                        return Application::EXIT_OK;
                    }
            
                    // Socket event
                    for( i = 0; i < n; i++ )
	                {
                        if( eventFD == events[i].data.fd )
	                    {
                            while( avObj.getEventQueue().getPostedCnt() )
                            {
                                HNAvahiBrowserEvent *event = (HNAvahiBrowserEvent*) avObj.getEventQueue().aquireRecord();

                                std::cout << "Browser Event - name: " << event->getName() << std::endl;

                                avObj.getEventQueue().releaseRecord( event );
                            }
                        }
                    }
                }

            }
            else if( _hnodeIDTest == true )
            {
                std::string rstStr;

                std::cout << "Running HNodeID test..." << std::endl;
                HNodeID idObj;

                std::cout << "TEST: Create" << std::endl;
                idObj.create();
                idObj.getStr( rstStr );
                std::cout << "SUCCESS: " << rstStr << std::endl;

                std::cout << "TEST: Set from string to 12:34:45:67:89:01:23:45:12:34:45:67:89:01:23:45" << std::endl;
                if( idObj.setFromStr( "12:34:45:67:89:01:23:45:12:34:45:67:89:01:23:45" ) == true )
                {
                    std::cout << "UNEXPECTED FAILURE" << std::endl;
                }
                else
                {
                    idObj.getStr( rstStr );
                    std::cout << "SUCCESS: " << rstStr << std::endl;
                }

                std::cout << "TEST: CRC32 string" << std::endl;
                std::cout << "SUCCESS: " << idObj.getCRC32AsHexStr() << std::endl;
            }
            else if( _hnodeConfigTest == true )
            {
                std::cout << "Running HNodeConfig test..." << std::endl;

                std::string rstStr;
                HNodeConfigFile cfgFile;
                HNodeConfig     cfg;

                HNCSection *secPtr;
                cfg.updateSection( "device", &secPtr );
                secPtr->updateValue( "test1", "value1" );

                cfg.updateSection( "owner", &secPtr );
                secPtr->updateValue( "test2", "value4" );

                cfg.updateSection( "device", &secPtr );
                secPtr->updateValue( "test3", "blah" );

                HNCObjList *listPtr;
                secPtr->updateList( "list-test", &listPtr );

                HNCObj *objPtr;
                listPtr->appendObj( &objPtr );
                objPtr->updateValue( "listv11", "foo" );
                objPtr->updateValue( "listv12", "bar" );
                
                listPtr->appendObj( &objPtr );
                objPtr->updateValue( "listv21", "zim" );
                objPtr->updateValue( "listv22", "zam" );
              
                cfg.debugPrint(2);

                cfgFile.setRootPath( "/tmp" );

    
                std::cout << "Saving config..." << std::endl;
                if( cfgFile.saveConfig( "TestCfgFile", "Inst1", cfg ) != HNC_RESULT_SUCCESS )
                {
                    std::cout << "ERROR: Could not save initial configuration." << std::endl;
                    return 1;
                }

                HNodeConfig cfg2;

                std::cout << "Loading config..." << std::endl;
                if( cfgFile.loadConfig( "TestCfgFile", "Inst1", cfg2 ) != HNC_RESULT_SUCCESS )
                {
                    std::cout << "ERROR: Could not load saved configuration." << std::endl;
                    return 1;
                }

                cfg2.debugPrint(2);
            }
            else if( _formatStrsTest == true )
            {
                std::cout << "Running HNFormatStrs test..." << std::endl;

                HNFormatStringStore  strStore;

                uint msgcode = 0;
                HNFS_RESULT_T result = strStore.registerFormatString( "Format Test string: %d, with %s, and 0x%2x, finally %1.2f", msgcode );

                std::cout << "registerFormatString result: " << result << " with message code: " << msgcode << std::endl;

                HNFSInstance instance;

                strStore.fillInstance( msgcode, instance, 20, "Test 1", 16, 4.556 );

                std::cout << "Instance Result Str: " << instance.getResultStr() << std::endl;
            }

            //std::cout << "We are now in main. Option is " << this->config().getString( "optionval" ) << std::endl;
            return 0;
        }
};

POCO_APP_MAIN( HNode2TestApp )

