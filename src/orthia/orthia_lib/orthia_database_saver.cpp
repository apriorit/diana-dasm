#include "orthia_database_saver.h"
#include "orthia_diana_module.h"

namespace orthia
{

CDatabaseSaver::CDatabaseSaver()
{
}

void CDatabaseSaver::Save(CDianaModule & dianaModule,
                          CDatabaseManager & databaseManager)
{
    orthia::intrusive_ptr<CDatabase> databaseModule = databaseManager.GetDatabase();
    Address_type baseAddress = dianaModule.GetModuleAddress();
    std::vector<CommonReferenceInfo> references;
    CDianaInstructionIterator iterator;
    dianaModule.QueryInstructionIterator(&iterator);

    CDatabaseModuleCleaner cleaner(databaseModule.get());
    databaseModule->StartSaveModule(baseAddress, dianaModule.GetModuleSize());

    while(!iterator.IsEmpty())
    {
        Address_type offset = iterator.GetInstructionOffset();
        iterator.QueryRefsToCurrentInstuction(&references);
        if (!references.empty())
        {
            // diana returns relative offsets, convert it to the absolute ones
            for(std::vector<CommonReferenceInfo>::iterator it = references.begin(), it_end = references.end();
                it != it_end; ++it)
            {
                if (!it->external)
                {
                    it->address += baseAddress;
                }
            }
            databaseModule->InsertReferencesToInstruction(offset+baseAddress, references);
        }
        iterator.QueryRefsFromCurrentInstruction(&references);
        if (!references.empty())
        {
            databaseModule->InsertReferencesFromInstruction(offset+baseAddress, references);    
        }
        iterator.MoveToNext();
    }
    databaseModule->DoneSave();
}

}