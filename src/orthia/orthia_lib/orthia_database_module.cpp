#include "orthia_database_module.h"
#include "orthia_interfaces.h"
#include "sqlite3.h"

namespace orthia
{

#define ORTHIA_CHECK_SQLITE(Expression, Text) { int orthia____code = (Expression); if (orthia____code != SQLITE_OK) { std::stringstream orthia____stream; orthia____stream<<"[SQLITE] "<<Text<<", code: "<<orthia____code; throw std::runtime_error(orthia____stream.str()); }} 
#define ORTHIA_CHECK_SQLITE2(Expression) ORTHIA_CHECK_SQLITE(Expression, "Error")

CSQLStatement::CSQLStatement(sqlite3_stmt * statement)
    : m_statement(statement)
{
}
CSQLStatement::~CSQLStatement()
{
    Finalize();
}
void CSQLStatement::Finalize()
{
    if (m_statement)
    {
        sqlite3_finalize(m_statement);
        m_statement = 0;
    }
}
void CSQLStatement::Reset(sqlite3_stmt * statement)
{
    if (m_statement)
        sqlite3_finalize(m_statement);
    m_statement = statement;
}
sqlite3_stmt ** CSQLStatement::Get2()
{
    Finalize();
    return &m_statement;
}
class CSQLAutoReset
{
    sqlite3_stmt * m_statement;

    CSQLAutoReset(const CSQLAutoReset&);
    CSQLAutoReset&operator =(const CSQLAutoReset&);
public:
    CSQLAutoReset(sqlite3_stmt * statement)
        :
            m_statement(statement)
    {
    }
    ~CSQLAutoReset()
    {
        sqlite3_reset(m_statement);
    }
};

CSQLDatabase::CSQLDatabase()
    :
        m_database(0)
{
}
CSQLDatabase::~CSQLDatabase()
{
    Reset(0);
}
void CSQLDatabase::Reset(sqlite3 * database)
{
    if (m_database)
    {
        sqlite3_close(m_database);
    }
    m_database = database;
}   
sqlite3 ** CSQLDatabase::Get2()
{
    Reset(0);
    return &m_database;
}
    
CDatabase::CDatabase()
{
}
CDatabase::~CDatabase()
{

}
void CDatabase::Init()
{
    // prepare references
    char * buffer = 0;
    buffer = "SELECT ref_address_from FROM tbl_references WHERE ref_address_to = ?1";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database.Get(), buffer, (int)strlen(buffer), m_stmtSelectReferencesTo.Get2(), NULL));
    buffer = "SELECT * FROM tbl_modules WHERE mod_address = ?1";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database.Get(), buffer, (int)strlen(buffer), m_stmtSelectModule.Get2(), NULL));
}
void CDatabase::OpenExisting(const std::wstring & fullFileName)
{
    ORTHIA_CHECK_SQLITE(sqlite3_open16(fullFileName.c_str(), m_database.Get2()), L"Can't create database");
    Init();
}
void CDatabase::CreateNew(const std::wstring & fullFileName)
{
    DeleteFile(fullFileName.c_str());
    ORTHIA_CHECK_SQLITE(sqlite3_open16(fullFileName.c_str(), m_database.Get2()), L"Can't create database");

    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database.Get(),"CREATE TABLE IF NOT EXISTS tbl_references (ref_address_from INTEGER, ref_address_to INTEGER)",
                        0,0,0), L"Can't create database");

    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database.Get(),"CREATE TABLE IF NOT EXISTS tbl_modules (mod_address INTEGER, mod_size INTEGER)",
                        0,0,0), L"Can't create database");
    Init();
}
void CDatabase::InsertReference(sqlite3_stmt * stmt, Address_type from, Address_type to)
{
    sqlite3_bind_int64(stmt, 1, from);
    sqlite3_bind_int64(stmt, 2, to);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        throw std::runtime_error("sqlite3_step failed");
    }
    sqlite3_reset(stmt);
}
void CDatabase::InsertModule(Address_type baseAddress, Address_type size)
{
    std::stringstream sql;
    sql<<"INSERT INTO tbl_modules VALUES("<<baseAddress<<","<<size<<")";
    std::string sqlString = sql.str();
    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database.Get(),sqlString.c_str(),
                0,0,0), L"Can't insert module");
}
void CDatabase::StartSaveModule(Address_type baseAddress, Address_type size)
{
    ORTHIA_CHECK_SQLITE2(sqlite3_exec(m_database.Get(), "BEGIN TRANSACTION", NULL, NULL, NULL));
    char * buffer = 0;
    buffer = "INSERT INTO tbl_references VALUES(?1, ?2)";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database.Get(), buffer, (int)strlen(buffer), m_stmtInsertReferences.Get2(), NULL));
    InsertModule(baseAddress, size);
}
void CDatabase::DoneSave()
{
    m_cache.clear();
    ORTHIA_CHECK_SQLITE2(sqlite3_exec(m_database.Get(), "COMMIT TRANSACTION", NULL, NULL, NULL));
}
void CDatabase::CleanupResources()
{
    m_stmtInsertReferences.Finalize();    
}
void CDatabase::InsertReferencesToInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references)
{
    for(std::vector<CommonReferenceInfo>::const_iterator it = references.begin(), it_end = references.end();
        it != it_end;
        ++it)
    {
        if (it->external)
        {
            throw std::runtime_error("External references are not supported");
        }
        InsertReference(m_stmtInsertReferences.Get(), it->address, offset); 
    }
}
void CDatabase::InsertReferencesFromInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references)
{
    for(std::vector<CommonReferenceInfo>::const_iterator it = references.begin(), it_end = references.end();
        it != it_end;
        ++it)
    {
        InsertReference(m_stmtInsertReferences.Get(), offset, it->address);
    }
}
void CDatabase::QueryReferencesToInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pReferences)
{
    CSQLAutoReset autoStatement(m_stmtSelectReferencesTo.Get());
    sqlite3_bind_int64(m_stmtSelectReferencesTo.Get(), 1, offset);
    pReferences->reserve(10);
    pReferences->clear();
    for(;;)
    {
        int stepResult = sqlite3_step(m_stmtSelectReferencesTo.Get());
        if (stepResult == SQLITE_DONE)
        {
            break;
        }
        else
        if (stepResult == SQLITE_ROW)
        {
            Address_type ref = sqlite3_column_int64(m_stmtSelectReferencesTo.Get(), 0);
            pReferences->push_back(CommonReferenceInfo(ref, false));
            continue;
        }
        throw std::runtime_error("sqlite3_step failed");
    }
}

void CDatabase::UnloadModule(Address_type offset)
{
    std::stringstream sql;
    sql<<"DELETE FROM tbl_modules WHERE mod_address = "<<offset;
    std::string sqlString = sql.str();
    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database.Get(), sqlString.c_str(),
                0,0,0), L"Can't unload module");
}
bool CDatabase::IsModuleExists(Address_type offset)
{
    CSQLAutoReset autoStatement(m_stmtSelectModule.Get());
    sqlite3_bind_int64(m_stmtSelectModule.Get(), 1, offset);
    int stepResult = sqlite3_step(m_stmtSelectModule.Get());
    if (stepResult == SQLITE_ROW)
    {
        return true;
    }
    return false;
}
void CDatabase::QueryModules(std::vector<CommonModuleInfo> * pResult)
{
    CSQLAutoReset autoStatement(m_stmtQueryModules.Get());
    pResult->reserve(10);
    pResult->clear();
    for(;;)
    {
        int stepResult = sqlite3_step(m_stmtQueryModules.Get());
        if (stepResult == SQLITE_DONE)
        {
            break;
        }
        else
        if (stepResult == SQLITE_ROW)
        {
            Address_type address = sqlite3_column_int64(m_stmtQueryModules.Get(), 0);
            pResult->push_back(CommonModuleInfo(address, L""));
            continue;
        }
        throw std::runtime_error("sqlite3_step failed");
    }
}

CDatabaseManager::CDatabaseManager()
{
}
CDatabaseManager::~CDatabaseManager()
{
}
 
// file api
void CDatabaseManager::CreateNew(const std::wstring & fullFileName)
{
    orthia::intrusive_ptr<CDatabase> database(new CDatabase());
    database->CreateNew(fullFileName);
    m_database = database;
}
void CDatabaseManager::OpenExisting(const std::wstring & fullFileName)
{
    orthia::intrusive_ptr<CDatabase> database(new CDatabase());
    database->OpenExisting(fullFileName);
    m_database = database;
}

orthia::intrusive_ptr<CDatabase> CDatabaseManager::GetDatabase()
{
    if (!m_database)
        throw std::runtime_error("No database initialized");
    return m_database;
}

}