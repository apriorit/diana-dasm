#include "orthia_module_manager.h"
#include "orthia_database_saver.h"

namespace orthia
{

CModuleManager::CModuleManager()
{
}
void CModuleManager::Reinit(const std::wstring & fullFileName,
                            bool bForce)
{
    orthia::intrusive_ptr<CDatabaseManager> pDatabaseManager(new CDatabaseManager());
    if (bForce || (!orthia::IsFileExist(fullFileName)) || !orthia::GetSizeOfFile(fullFileName))
    {
        pDatabaseManager->CreateNew(fullFileName);
    }
    else
    {
        pDatabaseManager->OpenExisting(fullFileName);
    }
    m_pDatabaseManager = pDatabaseManager;
    m_fullFileName = fullFileName;
}
std::wstring CModuleManager::GetDatabaseName()
{
    return m_fullFileName;
}

void CModuleManager::UnloadModule(Address_type offset)
{
    m_pDatabaseManager->GetDatabase()->UnloadModule(offset);
}
void CModuleManager::ReloadModule(Address_type offset,
                                  IMemoryReader * pMemoryReader,
                                  bool bForce)
{
    CDianaModule module;
    module.Init(offset, pMemoryReader);

    if (!m_pDatabaseManager)
        throw std::runtime_error("The profile is not initialized");
    
    // build full path name
    if (bForce || !m_pDatabaseManager->GetDatabase()->IsModuleExists(offset))
    {
        module.Analyze();
        m_pDatabaseManager->GetDatabase()->UnloadModule(offset);
        
        CDatabaseSaver fileSaver;
        fileSaver.Save(module, *m_pDatabaseManager);
    }
}

// module info
void CModuleManager::QueryLoadedModules(std::vector<CommonModuleInfo> * pResult)
{
    m_pDatabaseManager->GetDatabase()->QueryModules(pResult);
}
// references
void CModuleManager::QueryReferencesToInstruction(Address_type offset, 
                                                 std::vector<CommonReferenceInfo> * pResult)
{
    m_pDatabaseManager->GetDatabase()->QueryReferencesToInstruction(offset, pResult);
}

}