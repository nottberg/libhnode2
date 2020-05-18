#ifndef __HNODE_CONFIG_H__
#define __HNODE_CONFIG_H__

#include <string>
#include <vector>
#include <map>

#define HNCFG_DEFAULT_CFG_PATH  "/var/cache/hnode2/"
#define HNCFG_DEFAULT_CFG_FNAME "hnode2_config.json"

typedef enum HNCResultEnumeration
{
    HNC_RESULT_SUCCESS,
    HNC_RESULT_FAILURE
}HNC_RESULT_T;

class HNCObj
{
    private:
        std::map< std::string, std::string > pairMap;

    public:
        HNCObj();
       ~HNCObj();

        void clear();

        HNC_RESULT_T updateValue( std::string key, std::string value );

        void getValuePairs( std::map< std::string, std::string > &pairs );

        void debugPrint( uint offset );
};

class HNCObjList
{
    private:
        std::string name;

        std::vector< HNCObj > objList;

    public:
        HNCObjList( std::string name );
       ~HNCObjList();

        void clear();

        uint size();
        
        HNC_RESULT_T getObjPtr( uint index, HNCObj **objPtr );

        HNC_RESULT_T appendObj( HNCObj **objPtr );

        void debugPrint( uint offset );
};

class HNCSection
{
    private:
        std::string name;

        std::map< std::string, std::string > pairMap;

        std::map< std::string, HNCObjList > listMap;

    public:
        HNCSection( std::string name );
       ~HNCSection();

        void clear();

        HNC_RESULT_T updateValue( std::string key, std::string value );
        HNC_RESULT_T clearValue( std::string key );

        HNC_RESULT_T updateList( std::string name, HNCObjList **listObj );
        HNC_RESULT_T clearList( std::string name );

        std::string getName();

        HNC_RESULT_T getValueByName( std::string key, std::string &value );

        void getValuePairs( std::map< std::string, std::string > &pairs );
        void getListPairs( std::map< std::string, HNCObjList* > &pairs );

        void debugPrint( uint offset );
};

class HNodeConfig
{
    private:
        std::string baseName;
        std::string instanceName;

        std::map< std::string, HNCSection > sectionMap;

    public:
        HNodeConfig();
       ~HNodeConfig();

        void clear();

        HNC_RESULT_T updateSection( std::string name, HNCSection **secPtr );
        HNC_RESULT_T removeSection( std::string name );

        void getSectionPtrs( std::vector< HNCSection* > &sectionList );

        void debugPrint( uint offset );
};

class HNodeConfigFile
{
    private:
        std::string m_cfgFName;
        std::string m_rootPath;

        HNC_RESULT_T generatePathExtension( std::string baseName, std::string instanceName, std::string &pathExt );
        bool fileExists( std::string fPath );
        HNC_RESULT_T createDirectories( std::string dPath );
        HNC_RESULT_T createFile( std::string fPath );
        HNC_RESULT_T moveFile( std::string srcPath, std::string dstPath );

    public:
        HNodeConfigFile();
       ~HNodeConfigFile();

        void setRootPath( std::string value );
        std::string getRootPath();

        bool configExists( std::string baseName, std::string instanceName );

        HNC_RESULT_T loadConfig( std::string baseName, std::string instanceName, HNodeConfig &config );
        HNC_RESULT_T saveConfig( std::string baseName, std::string instanceName, HNodeConfig config );
};

#endif // __HNODE_CONFIG_H__
