#ifndef ORTHIA_MODULE_MANAGER_H
#define ORTHIA_MODULE_MANAGER_H

#include "orthia_utils.h"
#include "orthia_interfaces.h"

namespace orthia
{

struct IMemoryReader;
ORTHIA_PREDECLARE(class CDatabaseManager);

class CModuleManager
{
    CModuleManager(const CModuleManager&);
    CModuleManager&operator = (const CModuleManager&);

    orthia::intrusive_ptr<CDatabaseManager> m_pDatabaseManager;
    std::wstring m_fullFileName;
public:
    CModuleManager();
    void Reinit(const std::wstring & fullFileName, bool bForce);
    std::wstring GetDatabaseName();

    void UnloadModule(Address_type offset);
    void ReloadModule(Address_type offset,
                      IMemoryReader * pMemoryReader,
                      bool bForce);

    void QueryLoadedModules(std::vector<CommonModuleInfo> * pResult);
    // references
    void QueryReferencesToInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pResult);
};

}
#endif