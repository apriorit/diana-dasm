#include "orthia_database_saver.h"
#include "orthia_diana_module.h"

namespace orthia
{

CDatabaseSaver::CDatabaseSaver()
{
}
void CDatabaseSaver::Save(CDianaModule & dianaModule,
                          CDatabaseModule & databaseModule)
{
    std::vector<CDianaInstructionIterator::RefInfo> references;
    CDianaInstructionIterator iterator;
    dianaModule.QueryInstructionIterator(&iterator);
    while(!iterator.IsEmpty())
    {
        iterator.QueryRefsToCurrentInstuction(&references);
        iterator.QueryRefsFromCurrentInstruction(&references);
        iterator.MoveToNext();
    }
}

}