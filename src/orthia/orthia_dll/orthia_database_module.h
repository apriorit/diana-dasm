#ifndef ORTHIA_DATABASE_MODULE_H
#define ORTHIA_DATABASE_MODULE_H

#include "orthia_utils.h"

namespace orthia
{

class CDatabaseModule
{
public:
    CDatabaseModule();
    void CreateNew(const std::wstring & fullFileName);
};

}
#endif
