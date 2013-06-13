#ifndef ORTHIA_DATABASE_MODULE_H
#define ORTHIA_DATABASE_MODULE_H

#include "orthia_utils.h"

struct sqlite3;
struct sqlite3_stmt;
namespace orthia
{

struct CommonReferenceInfo;
class CDatabaseModule:public orthia::RefCountedBase
{
    std::set<Address_type> m_cache;
    CDatabaseModule(const CDatabaseModule &);
    CDatabaseModule & operator =(const CDatabaseModule &);
    sqlite3 * m_database;
    sqlite3_stmt * m_stmtInsertInstructions;
    sqlite3_stmt * m_stmtInsertReferences;
    sqlite3_stmt * m_stmtInsertExternalReferences;
    sqlite3_stmt * m_stmtSelectReferencesTo;

    void InsertReference(sqlite3_stmt * stmt, Address_type from, Address_type to);

public:
    CDatabaseModule();
    ~CDatabaseModule();
    void CreateNew(const std::wstring & fullFileName);
    void OpenExisting(const std::wstring & fullFileName);

    void StartSave();
    void DoneSave();
    void CleanupResources();

    void InsertInstruction(Address_type offset);
    void InsertReferencesToInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references);
    void InsertReferencesFromInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references);

    void QueryReferencesToInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pReferences);
    void QueryReferencesFromInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pReferences);
};

ORTHIA_DECLARE(CDatabaseModule);
class CDatabaseModuleCleaner
{
    CDatabaseModule * m_pDatabaseModule;
    CDatabaseModuleCleaner(const CDatabaseModuleCleaner&);
    CDatabaseModuleCleaner & operator = (const CDatabaseModuleCleaner&);
public:
    CDatabaseModuleCleaner(CDatabaseModule * pDatabaseModule)
        :
            m_pDatabaseModule(pDatabaseModule)
    {
    }
    ~CDatabaseModuleCleaner()
    {
        m_pDatabaseModule->CleanupResources();
    }
};

}
#endif
