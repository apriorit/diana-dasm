#ifndef ORTHIA_DATABASE_MODULE_H
#define ORTHIA_DATABASE_MODULE_H

#include "orthia_utils.h"
#include "orthia_interfaces.h"

struct sqlite3;
struct sqlite3_stmt;
namespace orthia
{

class CSQLStatement
{
    CSQLStatement(const CSQLStatement&);
    CSQLStatement&operator = (const CSQLStatement&);
    sqlite3_stmt * m_statement;
public:
    CSQLStatement(sqlite3_stmt * statement=0);
    ~CSQLStatement();
    void Finalize();
    void Reset(sqlite3_stmt * statement);
    sqlite3_stmt * Get() { return m_statement; }
    sqlite3_stmt ** Get2();
};
struct CommonReferenceInfo;

class CSQLDatabase
{
    CSQLDatabase(const CSQLDatabase &);
    CSQLDatabase&operator =(const CSQLDatabase &);
    sqlite3 * m_database;
public:
    CSQLDatabase();
    ~CSQLDatabase();
    void Reset(sqlite3 * database);
    sqlite3 * Get() { return m_database; }
    sqlite3 ** Get2();
};

class CDatabase:public orthia::RefCountedBase
{
    std::set<Address_type> m_cache;
    CDatabase(const CDatabase &);
    CDatabase & operator =(const CDatabase &);

    CSQLDatabase m_database;
    CSQLStatement m_stmtInsertReferences;
    CSQLStatement m_stmtSelectReferencesTo;
    CSQLStatement m_stmtSelectModule;
    CSQLStatement m_stmtQueryModules;

    void InsertReference(sqlite3_stmt * stmt, Address_type from, Address_type to);
    void InsertModule(Address_type baseAddress, Address_type size);

    void Init();
public:
    CDatabase();
    ~CDatabase();
    // file api
    void CreateNew(const std::wstring & fullFileName);
    void OpenExisting(const std::wstring & fullFileName);

    // module loading process:
    void StartSaveModule(Address_type baseAddress, Address_type size);
    void DoneSave();
    void CleanupResources();

    void InsertReferencesToInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references);
    void InsertReferencesFromInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references);

    // queries
    void QueryReferencesToInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pReferences);
    
    // modules api
    void UnloadModule(Address_type offset);
    bool IsModuleExists(Address_type offset);
    void QueryModules(std::vector<CommonModuleInfo> * pResult);
};

class CDatabaseModuleCleaner
{
    CDatabase * m_pDatabaseModule;
    CDatabaseModuleCleaner(const CDatabaseModuleCleaner&);
    CDatabaseModuleCleaner & operator = (const CDatabaseModuleCleaner&);
public:
    CDatabaseModuleCleaner(CDatabase * pDatabaseModule)
        :
            m_pDatabaseModule(pDatabaseModule)
    {
    }
    ~CDatabaseModuleCleaner()
    {
        m_pDatabaseModule->CleanupResources();
    }
};

class CDatabaseManager:public orthia::RefCountedBase
{
    CDatabaseManager(const CDatabaseManager &);
    CDatabaseManager & operator =(const CDatabaseManager &);

    orthia::intrusive_ptr<CDatabase> m_database;
public:
    CDatabaseManager();
    ~CDatabaseManager();
    // file api
    void CreateNew(const std::wstring & fullFileName);
    void OpenExisting(const std::wstring & fullFileName);

    orthia::intrusive_ptr<CDatabase> GetDatabase();

 
};

ORTHIA_DECLARE(CDatabase);
ORTHIA_DECLARE(CDatabaseManager);

}
#endif
