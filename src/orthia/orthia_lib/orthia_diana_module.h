#ifndef ORTHIA_DIANA_MODULE_H
#define ORTHIA_DIANA_MODULE_H

#include "orthia_interfaces.h"

namespace orthia
{

class CDianaModuleImpl;
class CDianaModule
{
    Address_type m_offset;
    std::wstring m_uniqueName;
    IMemoryReader * m_pMemoryReader;
    std::auto_ptr<CDianaModuleImpl> m_impl;
public:
    CDianaModule();
    ~CDianaModule();
    void Init(Address_type offset,
              IMemoryReader * pMemoryReader);

    void Analyze();
    std::wstring GetUniqueName() const;
    std::wstring GetName() const;
};

}
#endif
