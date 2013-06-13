#include "orthia_database_module.h"
#include "orthia_interfaces.h"
#include "sqlite3.h"

namespace orthia
{

#define ORTHIA_CHECK_SQLITE(Expression, Text) { int orthia____code = (Expression); if (orthia____code != SQLITE_OK) { std::stringstream orthia____stream; orthia____stream<<"[SQLITE] "<<Text<<", code: "<<orthia____code; throw std::runtime_error(orthia____stream.str()); }} 
#define ORTHIA_CHECK_SQLITE2(Expression) ORTHIA_CHECK_SQLITE(Expression, "Error")

CDatabaseModule::CDatabaseModule()
    :
        m_database(0),
        m_stmtInsertInstructions(0),
        m_stmtInsertReferences(0),
        m_stmtInsertExternalReferences(0)
{
}
CDatabaseModule::~CDatabaseModule()
{
    if (m_stmtSelectReferencesTo)
    {
        sqlite3_finalize(m_stmtSelectReferencesTo);
    }
    if (m_database)
    {
        sqlite3_close(m_database);
    }
}
void CDatabaseModule::OpenExisting(const std::wstring & fullFileName)
{
    ORTHIA_CHECK_SQLITE(sqlite3_open16(fullFileName.c_str(), &m_database), L"Can't create database");
}
void CDatabaseModule::CreateNew(const std::wstring & fullFileName)
{
    DeleteFile(fullFileName.c_str());
    ORTHIA_CHECK_SQLITE(sqlite3_open16(fullFileName.c_str(), &m_database), L"Can't create database");

    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database,"CREATE TABLE IF NOT EXISTS tbl_instructions (inst_address INTEGER PRIMARY KEY)",
                        0,0,0), L"Can't create database");

    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database,"CREATE TABLE IF NOT EXISTS tbl_references (ref_address_from INTEGER, ref_address_to INTEGER, FOREIGN KEY(ref_address_from) REFERENCES tbl_instructions(inst_address), FOREIGN KEY(ref_address_to) REFERENCES tbl_instructions(inst_address))",
                        0,0,0), L"Can't create database");

    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database,"CREATE TABLE IF NOT EXISTS tbl_external_references (ref_address_from INTEGER, ref_address_to INTEGER, FOREIGN KEY(ref_address_from) REFERENCES tbl_instructions(inst_address))",
                        0,0,0), L"Can't create database");
}
void CDatabaseModule::InsertInstruction(Address_type offset)
{
    if (!m_cache.insert(offset).second)
        return;

    sqlite3_bind_int64(m_stmtInsertInstructions, 1, offset);
    if (sqlite3_step(m_stmtInsertInstructions) != SQLITE_DONE)
    {
        throw std::runtime_error("sqlite3_step failed");
    }
    sqlite3_reset(m_stmtInsertInstructions);
}
void CDatabaseModule::InsertReference(sqlite3_stmt * stmt, Address_type from, Address_type to)
{
    sqlite3_bind_int64(stmt, 1, from);
    sqlite3_bind_int64(stmt, 2, to);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        throw std::runtime_error("sqlite3_step failed");
    }
    sqlite3_reset(stmt);
}
void CDatabaseModule::StartSave()
{
    ORTHIA_CHECK_SQLITE2(sqlite3_exec(m_database, "BEGIN TRANSACTION", NULL, NULL, NULL));
    char * buffer = "INSERT INTO tbl_instructions VALUES(?1)";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database, buffer, strlen(buffer), &m_stmtInsertInstructions, NULL));
    buffer = "INSERT INTO tbl_references VALUES(?1, ?2)";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database, buffer, strlen(buffer), &m_stmtInsertReferences, NULL));
    buffer = "INSERT INTO tbl_external_references VALUES(?1, ?2)";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database, buffer, strlen(buffer), &m_stmtInsertExternalReferences, NULL));
    buffer = "SELECT ref_address_from FROM tbl_references WHERE ref_address_to = ?1";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database, buffer, strlen(buffer), &m_stmtSelectReferencesTo, NULL));
}
void CDatabaseModule::DoneSave()
{
    m_cache.clear();
    ORTHIA_CHECK_SQLITE2(sqlite3_exec(m_database, "COMMIT TRANSACTION", NULL, NULL, NULL));
}
void CDatabaseModule::CleanupResources()
{
    if (m_stmtInsertInstructions)
    {
        sqlite3_finalize(m_stmtInsertInstructions);
    }
    if (m_stmtInsertReferences)
    {
        sqlite3_finalize(m_stmtInsertReferences);
    }
    if (m_stmtInsertExternalReferences)
    {
        sqlite3_finalize(m_stmtInsertExternalReferences);
    }
}
void CDatabaseModule::InsertReferencesToInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references)
{
    for(std::vector<CommonReferenceInfo>::const_iterator it = references.begin(), it_end = references.end();
        it != it_end;
        ++it)
    {
        if (it->external)
        {
            throw std::runtime_error("External references are not supported");
        }
        InsertReference(m_stmtInsertReferences, it->address, offset); 
    }
}
void CDatabaseModule::InsertReferencesFromInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references)
{
    for(std::vector<CommonReferenceInfo>::const_iterator it = references.begin(), it_end = references.end();
        it != it_end;
        ++it)
    {
        if (!it->external)
        {
            InsertInstruction(it->address);
            InsertReference(m_stmtInsertReferences, offset, it->address); 
            continue;
        }
        InsertReference(m_stmtInsertExternalReferences, offset, it->address); 
    }
}
void CDatabaseModule::QueryReferencesToInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pReferences)
{
    sqlite3_bind_int64(m_stmtSelectReferencesTo, 1, offset);
    pReferences->reserve(10);
    pReferences->clear();
    for(;;)
    {
        int stepResult = sqlite3_step(m_stmtSelectReferencesTo);
        if (stepResult == SQLITE_DONE)
        {
            break;
        }
        else
        if (stepResult == SQLITE_ROW)
        {
            Address_type ref = sqlite3_column_int64(m_stmtSelectReferencesTo, 0);
            pReferences->push_back(CommonReferenceInfo(ref, false));
            continue;
        }
        throw std::runtime_error("sqlite3_step failed");
    }
    sqlite3_reset(m_stmtSelectReferencesTo);
}
void CDatabaseModule::QueryReferencesFromInstruction(Address_type offset, std::vector<CommonReferenceInfo> * pReferences)
{
}


}