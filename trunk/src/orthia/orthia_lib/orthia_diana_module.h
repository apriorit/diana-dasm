#ifndef ORTHIA_DIANA_MODULE_H
#define ORTHIA_DIANA_MODULE_H

#include "orthia_interfaces.h"

namespace orthia
{


class CDianaModule;
class CDianaInstructionIterator
{
    int m_currentInstruction;
    CDianaModule * m_pModule;
public:
    struct RefInfo
    {
        Address_type address;
        bool external;
        RefInfo(Address_type address_in,
                bool external_in)
          :
            address(address_in),
            external(external_in)
        {
        }
    };
    CDianaInstructionIterator();
    void Init(CDianaModule * pModule);
    void MoveToFirst();
    void MoveToNext();
    bool IsEmpty() const;
    Address_type GetInstructionOffset();
    void QueryRefsToCurrentInstuction(std::vector<RefInfo> * pInfo);
    void QueryRefsFromCurrentInstruction(std::vector<RefInfo> * pInfo);
};

class CDianaModuleImpl;
class CDianaModule
{
    friend class CDianaInstructionIterator;
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

    void QueryInstructionIterator(CDianaInstructionIterator * pIterator);
};

}
#endif
