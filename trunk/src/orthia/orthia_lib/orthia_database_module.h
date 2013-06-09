#ifndef ORTHIA_DATABASE_MODULE_H
#define ORTHIA_DATABASE_MODULE_H

#include "orthia_utils.h"

struct sqlite3;
namespace orthia
{

class CDatabaseModule
{
    CDatabaseModule(const CDatabaseModule &);
    CDatabaseModule & operator =(const CDatabaseModule &);
    sqlite3 * m_database;
public:
    CDatabaseModule();
    ~CDatabaseModule();
    void CreateNew(const std::wstring & fullFileName);
};

}
#endif
