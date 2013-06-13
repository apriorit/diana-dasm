#ifndef ORTHIA_MODULE_MANAGER_H
#define ORTHIA_MODULE_MANAGER_H

#include "orthia_utils.h"
#include "orthia_interfaces.h"

namespace orthia
{

struct IMemoryReader;
ORTHIA_PREDECLARE(class CDatabaseModule);

class CModuleManager
{
    CModuleManager(const CModuleManager&);
    CModuleManager&operator = (const CModuleManager&);

    struct ModuleInfoInternal
    {
        std::wstring m_name;
        std::wstring m_fullDatabaseName;
        orthia::intrusive_ptr<CDatabaseModule> m_pDatabaseModule;
        Address_type m_moduleAddress;
        Address_type m_moduleSize;
        ModuleInfoInternal();
        ModuleInfoInternal(const std::wstring & name,
                           const std::wstring & fullDatabaseName,
                           orthia::intrusive_ptr<CDatabaseModule> pDatabaseModule,
                           Address_type moduleAddress,
                           Address_type moduleSize);
    };
    typedef std::map<Address_type, ModuleInfoInternal> ModuleMap_type;
    ModuleMap_type m_modules;
    std::wstring m_path;

    ModuleInfoInternal * QueryModuleInfo(Address_type offset);
public:
    CModuleManager();
    void Reinit(const std::wstring & path);
    std::wstring GetPath();

    void UnloadModule(Address_type offset);
    void ReloadModule(Address_type offset,
                      IMemoryReader * pMemoryReader,
                      bool bForce);

    // module info
    struct ModuleInfo
    {
        Address_type address;
        std::wstring name;
    };
    void QueryLoadedModules(std::vector<ModuleInfo> * pResult);
    // references
    void QueryReferencesToInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pResult);
};

}
#endif