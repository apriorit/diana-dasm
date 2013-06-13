#include "orthia_module_manager.h"
#include "orthia_database_saver.h"

namespace orthia
{

CModuleManager::ModuleInfoInternal::ModuleInfoInternal()
:
m_moduleAddress(0),
m_moduleSize(0)
{
}
CModuleManager::ModuleInfoInternal::ModuleInfoInternal(const std::wstring & name,
                   const std::wstring & fullDatabaseName,
                   orthia::intrusive_ptr<CDatabaseModule> pDatabaseModule,
                   Address_type moduleAddress,
                   Address_type moduleSize)
    :
        m_name(name),
        m_fullDatabaseName(fullDatabaseName),
        m_pDatabaseModule(pDatabaseModule),
        m_moduleAddress(moduleAddress),
        m_moduleSize(moduleSize)
{
}

CModuleManager::CModuleManager()
{
}
void CModuleManager::Reinit(const std::wstring & path)
{
    m_path = path;
}
std::wstring CModuleManager::GetPath()
{
    return m_path;
}

void CModuleManager::UnloadModule(Address_type offset)
{
    m_modules.erase(offset);
}
void CModuleManager::ReloadModule(Address_type offset,
                                  IMemoryReader * pMemoryReader,
                                  bool bForce)
{
    CDianaModule module;
    module.Init(offset, pMemoryReader);

    // build full path name
    std::wstring fullFileName;
    fullFileName = m_path + L"\\" + module.GetUniqueName();

    orthia::intrusive_ptr<CDatabaseModule> pDatabaseModule(new CDatabaseModule());
    if (bForce || (!orthia::IsFileExist(fullFileName)) || !orthia::GetSizeOfFile(fullFileName))
    {
        module.Analyze();

        m_modules.erase(offset);
        orthia::CreateAllDirectoriesForFile(fullFileName);
        pDatabaseModule->CreateNew(fullFileName);

        CDatabaseSaver fileSaver;
        fileSaver.Save(module, *pDatabaseModule);
    }
    else
    {
        pDatabaseModule->OpenExisting(fullFileName);
    }
    m_modules[offset] = ModuleInfoInternal(module.GetName(), fullFileName, pDatabaseModule, offset, module.GetModuleSize());
}

// module info
void CModuleManager::QueryLoadedModules(std::vector<ModuleInfo> * pResult)
{
}
// references
void CModuleManager::QueryReferencesToInstruction(Address_type offset, 
                                                 std::vector<CommonReferenceInfo> * pResult)
{
    ModuleInfoInternal * pInfo = QueryModuleInfo(offset);
    pInfo->m_pDatabaseModule->QueryReferencesToInstruction(offset - pInfo->m_moduleAddress, pResult);
    for(std::vector<CommonReferenceInfo>::iterator it = pResult->begin(), it_end = pResult->end();
        it != it_end;
        ++it)
    {
        it->address += pInfo->m_moduleAddress;
    }
}
CModuleManager::ModuleInfoInternal * CModuleManager::QueryModuleInfo(Address_type offset)
{
    ModuleMap_type::iterator it, it_end;
    for(it = m_modules.begin(), it_end = m_modules.end();
        it != it_end;
        ++it)
    {
        if (it->first <= offset && offset <= it->first + it->second.m_moduleSize)
        {
            break;
        }
    }
    if (it == m_modules.end())
    {
        ORTHIA_THROW_STD("Module with address ["<<offset<<"] not found");
    }
    return &it->second;
}

}