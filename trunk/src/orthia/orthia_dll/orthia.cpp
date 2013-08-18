#include "orthia.h"
#include "orthia_module_manager.h"
#include "orthia_memory_cache.h"
#include "diana_core_cpp.h"


class ÑWindbgMemoryReader:public orthia::IMemoryReader
{
public:
    virtual void Read(orthia::Address_type offset, 
                      orthia::Address_type bytesToRead,
                      void * pBuffer,
                      orthia::Address_type * pBytesRead)
    {
        if (bytesToRead >= (orthia::Address_type)ULONG_MAX)
        {
            std::stringstream text;
            text<<"Range size is too big: "<<bytesToRead;
            throw std::runtime_error(text.str());
        }
        ULONG bytes = 0;
        ReadMemory(offset, 
                   pBuffer, 
                   (ULONG)bytesToRead,
                   &bytes);
        *pBytesRead = bytes;
    }
};

struct ModuleManagerObjects
{
    ÑWindbgMemoryReader reader;
    orthia::CModuleManager moduleManager;
    ModuleManagerObjects()
    {
    }
    orthia::IMemoryReader * GetReader()
    {
        return &reader;
    }
};
static std::auto_ptr<ModuleManagerObjects> g_globals;


namespace orthia
{

static void PrintUsage()
{
    dprintf("!help - display this text\n");
    dprintf("!profile [/f] <full_name> - use/create profile\n");
    dprintf("!lm - displays module list\n");
    dprintf("!reload [/u] [/v] [/f] <module_address> - reloads module (u - unload, v - verbose, f - force)\n");
    dprintf("!x <address> - prints xrefs\n");
}
static void SetupPath(const std::wstring & path, bool bForce)
{
    if (!g_globals.get())
    {
        g_globals.reset(new ModuleManagerObjects);
    }
    g_globals->moduleManager.Reinit(path, bForce);
}

orthia::IMemoryReader * QueryReader()
{
    ModuleManagerObjects * pGlobal = g_globals.get();
    return pGlobal->GetReader();
}
orthia::CModuleManager * QueryModuleManager()
{
    ModuleManagerObjects * pGlobal = g_globals.get();
    if (!pGlobal) 
    {
        throw std::runtime_error("Profile not inited");
    }
    return &pGlobal->moduleManager;
}

void InitLib()
{
    Diana_Init();
    try
    {
        // TODO: add initialization code here

    }
    catch(const std::exception & e)
    {
        &e;
        // do not care;
    }
}

} // orthia namespace

// commands
ORTHIA_DECLARE_API(help)
{
    dprintf("Orthia interface:\n\n");
    orthia::PrintUsage();
    return S_OK;
}

ORTHIA_DECLARE_API(profile)
{
    ORTHIA_CMD_START

        bool bForce = false;
        std::wstring wargs = orthia::ToString(orthia::Trim(args));
        if (wcsncmp(wargs.c_str(), L"/f ", 3) == 0)
        {
            bForce = true;
            wargs.erase(0, 3);
            wargs = orthia::Trim(wargs);
        }
        std::wstring path = orthia::ExpandVariable(wargs);
        orthia::SetupPath(path, bForce);

    ORTHIA_CMD_END
}

ORTHIA_DECLARE_API(lm)
{
    ORTHIA_CMD_START
        
    orthia::CModuleManager * pModuleManager = orthia::QueryModuleManager();
    std::vector<orthia::CommonModuleInfo> modules;
    pModuleManager->QueryLoadedModules(&modules);
    for(std::vector<orthia::CommonModuleInfo>::iterator it = modules.begin(), it_end = modules.end();
        it != it_end;
        ++it)
    {
        dprintf("%I64lx %s\n", it->address, it->name.c_str());
    }   

    ORTHIA_CMD_END
}

ORTHIA_DECLARE_API(reload)
{
    ORTHIA_CMD_START
        
    std::vector<std::wstring> words;
    orthia::Split(orthia::ToString(args), &words);
    bool bUnload = false;
    orthia::Address_type offset = 0;
    bool offsetInited = false;
    bool bForce = false;
    bool bVerbose = false;
    for(std::vector<std::wstring>::iterator it = words.begin(), it_end = words.end();
        it != it_end;
        ++it)
    {
        if (*it== L"/u")
        {
            bUnload = true;
            continue;
        }
        if (*it== L"/f")
        {
            bForce = true;
            continue;
        }
        if (*it== L"/v")
        {
            bVerbose = true;
            continue;
        }
        if (offsetInited)
            throw std::runtime_error("Unexpected argument: " + orthia::ToAnsiString_Silent(*it));
        PCSTR tail = 0;
        std::string strOffset = orthia::ToAnsiString_Silent(*it);
        GetExpressionEx(strOffset.c_str(), &offset, &tail);
        offsetInited = true;
    }
    
    orthia::CModuleManager * pModuleManager = orthia::QueryModuleManager();
    if (bUnload)
    {
        pModuleManager->UnloadModule(offset);
        if (bVerbose)
        {
            dprintf("%s %I64lx\n", "Module unloaded: ", offset);
        }
        return S_OK;
    }
    pModuleManager->ReloadModule(offset, orthia::QueryReader(), bForce);
    if (bVerbose)    
    {    
        dprintf("%s %I64lx\n", "Module loaded: ", offset);     
    }
    ORTHIA_CMD_END
}

ORTHIA_DECLARE_API(x)
{
    ORTHIA_CMD_START

    // read parameters
    ULONG64 address = 0;
    
    if (!GetExpressionEx(args, &address, &args)) 
    {
        throw std::runtime_error("Address expression expected");
    }

    orthia::CModuleManager * pModuleManager = orthia::QueryModuleManager();

    std::vector<orthia::CommonReferenceInfo> references;
    pModuleManager->QueryReferencesToInstruction(address, &references);

    for(std::vector<orthia::CommonReferenceInfo>::iterator it = references.begin(), it_end = references.end();
        it != it_end;
        ++it)
    {
        dprintf("%I64lx\n", it->address);
    }   
    ORTHIA_CMD_END
}
