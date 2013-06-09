#include "orthia_database_module.h"
#include "sqlite3.h"

namespace orthia
{

#define ORTHIA_CHECK_SQLITE(Expression, Text) { int orthia____code = (Expression); if (orthia____code != SQLITE_OK) { std::stringstream orthia____stream; orthia____stream<<"[SQLITE] "<<Text<<", code: "<<orthia____code; throw std::runtime_error(orthia____stream.str()); }} 

CDatabaseModule::CDatabaseModule()
    :
        m_database(0)
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
}


}