#ifndef _HN_HTTP_SERVER_H_
#define _HN_HTTP_SERVER_H_

#include <string>
#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Util/OptionSet.h"

using Poco::Util::OptionSet;

class HNHttpServer: public Poco::Util::ServerApplication
{
    public:
        HNHttpServer();
        ~HNHttpServer();

    protected:
        void initialize(Application& self);

        void uninitialize();

        void defineOptions(OptionSet& options);

        void handleHelp(const std::string& name, const std::string& value);

        int main(const std::vector<std::string>& args);

    private:
        bool _helpRequested;
};

#endif
