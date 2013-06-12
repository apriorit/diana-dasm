#include "orthia_database_module.h"
#include "sqlite3.h"

namespace orthia
{

#define ORTHIA_CHECK_SQLITE(Expression, Text) { int orthia____code = (Expression); if (orthia____code != SQLITE_OK) { std::stringstream orthia____stream; orthia____stream<<"[SQLITE] "<<Text<<", code: "<<orthia____code; throw std::runtime_error(orthia____stream.str()); }} 
#define ORTHIA_CHECK_SQLITE2(Expression) ORTHIA_CHECK_SQLITE(Expression, "Error")

CDatabaseModule::CDatabaseModule()
    :
        m_database(0),
        m_stmtInsertInstructions(0)
{
}
CDatabaseModule::~CDatabaseModule()
{
    if (m_database)
    {
        sqlite3_close(m_database);
    }
}
void CDatabaseModule::CreateNew(const std::wstring & fullFileName)
{
    ORTHIA_CHECK_SQLITE(sqlite3_open16(fullFileName.c_str(), &m_database), L"Can't create database");

    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database,"CREATE TABLE IF NOT EXISTS tbl_instructions (inst_address INTEGER PRIMARY KEY)",
                        0,0,0), L"Can't create database");

    ORTHIA_CHECK_SQLITE(sqlite3_exec(m_database,"CREATE TABLE IF NOT EXISTS tbl_references (ref_address_from INTEGER, ref_address_to INTEGER, FOREIGN KEY(ref_address_from) REFERENCES tbl_instructions(inst_address), FOREIGN KEY(ref_address_to) REFERENCES tbl_instructions(inst_address))",
                        0,0,0), L"Can't create database");
}
void CDatabaseModule::InsertInstruction(Address_type offset)
{
    sqlite3_bind_int64(m_stmtInsertInstructions, 1, offset);
    if (sqlite3_step(m_stmtInsertInstructions) != SQLITE_DONE)
    {
        throw std::runtime_error("sqlite3_step failed");
    }
    sqlite3_reset(m_stmtInsertInstructions);
}

void CDatabaseModule::StartSave()
{
    ORTHIA_CHECK_SQLITE2(sqlite3_exec(m_database, "BEGIN TRANSACTION", NULL, NULL, NULL));
    char * buffer = "INSERT INTO tbl_instructions VALUES(?1)";
    ORTHIA_CHECK_SQLITE2(sqlite3_prepare_v2(m_database, buffer, strlen(buffer), &m_stmtInsertInstructions, NULL));
}
void CDatabaseModule::DoneSave()
{
    ORTHIA_CHECK_SQLITE2(sqlite3_exec(m_database, "COMMIT TRANSACTION", NULL, NULL, NULL));
}
void CDatabaseModule::CleanupResources()
{
    sqlite3_finalize(m_stmtInsertInstructions);
}
void CDatabaseModule::InsertReferencesToInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references)
{
}
void CDatabaseModule::InsertReferencesFromInstruction(Address_type offset, const std::vector<CommonReferenceInfo> & references)
{
}


}