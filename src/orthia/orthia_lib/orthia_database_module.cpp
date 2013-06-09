#include "orthia_database_module.h"
#include "sqlite3.h"

namespace orthia
{

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
    sqlite3_open16(fullFileName.c_str(), &m_database);
}


}