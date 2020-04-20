#if 0
#include "HNHttpServer.h"

int 
main( int argc, char** argv )
{
    HNHttpServer app;
    return app.run(argc, argv);
}
#endif

#include <unistd.h>

#include <Poco/Util/Application.h>

#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionCallback.h>

#include <Poco/Util/HelpFormatter.h>

#include <iostream>

#include "HNHttpServer.h"
#include "HNAvahi.h"
#include "HNodeID.h"
#include "HNodeConfig.h"

class HNode2TestApp : public Poco::Util::Application
{
    private:
        bool _helpRequested;
        bool _httpServerTest;
        bool _avahiTest;
        bool _hnodeIDTest;
        bool _hnodeConfigTest;

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
            _helpRequested   = false;
            _httpServerTest  = false;
            _avahiTest       = false;
            _hnodeIDTest     = false;
            _hnodeConfigTest = false;

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
                Poco::Util::Option( "hnodeid", "i", "Run HNodeID test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
            );

            optionSet.addOption(
                Poco::Util::Option( "hnodecfg", "c", "Run HNodeConfig test." ).callback( Poco::Util::OptionCallback<HNode2TestApp>(this, &HNode2TestApp::handleTest ) )
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
            else if( name == "hnodeid" )
                _hnodeIDTest = true;
            else if( name == "hnodecfg" )
                _hnodeConfigTest = true;

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

            //std::cout << "We are now in main. Option is " << this->config().getString( "optionval" ) << std::endl;
            return 0;
        }
};

POCO_APP_MAIN( HNode2TestApp )

