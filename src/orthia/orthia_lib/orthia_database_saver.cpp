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
    std::vector<CommonReferenceInfo> references;
    CDianaInstructionIterator iterator;
    dianaModule.QueryInstructionIterator(&iterator);

    CDatabaseModuleCleaner cleaner(&databaseModule);
    databaseModule.StartSave();

    while(!iterator.IsEmpty())
    {
        Address_type offset = iterator.GetInstructionOffset();
        iterator.QueryRefsToCurrentInstuction(&references);
        bool bInstructionAdded = false;
        if (!references.empty())
        {
            databaseModule.InsertInstruction(offset);
            bInstructionAdded = true;
            databaseModule.InsertReferencesToInstruction(offset, references);
        }
        iterator.QueryRefsFromCurrentInstruction(&references);
        if (!references.empty())
        {
            if (!bInstructionAdded)
            {
                databaseModule.InsertInstruction(offset);
            }
            databaseModule.InsertReferencesFromInstruction(offset, references);    
        }
        iterator.MoveToNext();
    }
    databaseModule.DoneSave();
}

}