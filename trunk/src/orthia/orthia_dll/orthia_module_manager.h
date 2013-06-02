#ifndef ORTHIA_MODULE_MANAGER_H
#define ORTHIA_MODULE_MANAGER_H

#include "orthia_utils.h"

namespace orthia
{

struct IMemoryReader;

class CModuleManager
{
    CModuleManager(const CModuleManager&);
    CModuleManager&operator = (const CModuleManager&);

    struct ModuleInfoInternal
    {
        std::wstring m_name;
        std::wstring m_fullDatabaseName;
        ModuleInfoInternal()
        {
        }
        ModuleInfoInternal(const std::wstring & name,
                           const std::wstring & fullDatabaseName)
            :
                m_name(name),
                m_fullDatabaseName(fullDatabaseName)
        {
        }
    };
    typedef std::map<Address_type, ModuleInfoInternal> ModuleMap_type;
    ModuleMap_type m_modules;
    std::wstring m_path;
public:
    CModuleManager();
    void Reinit(const std::wstring & path);
    std::wstring GetPath();

    void UnloadModule(Address_type offset);
    void ReloadModule(Address_type offset,
                      IMemoryReader * pMemoryReader);

    // module info
    struct ModuleInfo
    {
        Address_type address;
        std::wstring name;
    };
    void QueryLoadedModules(std::vector<ModuleInfo> * pResult);
    // references
    struct ReferenceInfo
    {
        Address_type address;
    };
    void QueryReferences(Address_type offset, std::vector<ReferenceInfo> * pResult);
};

}
#endif