#include "orthia_module_manager.h"
#include "orthia_database_saver.h"

namespace orthia
{

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
                                  IMemoryReader * pMemoryReader)
{
    CDianaModule module;
    module.Init(offset, pMemoryReader);

    std::wstring fullFileName;
    fullFileName = m_path + module.GetUniqueName();
    std::wstring moduleName = module.GetName();
    CDatabaseModule databaseModule;
    databaseModule.CreateNew(fullFileName);

    CDatabaseSaver fileSaver;
    fileSaver.Save(module, databaseModule);
    m_modules[offset] = ModuleInfoInternal(moduleName, fullFileName);
}

// module info
void CModuleManager::QueryLoadedModules(std::vector<ModuleInfo> * pResult)
{
}
// references
void CModuleManager::QueryReferences(Address_type offset, 
                                     std::vector<ReferenceInfo> * pResult)
{
}

}