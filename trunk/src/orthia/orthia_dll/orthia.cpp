#include "orthia.h"
#include "orthia_module_manager.h"
#include "orthia_memory_cache.h"

#define GLOBAL_ORTHIA_PATH_ANSI "%ORTHIA_PATH%"

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
static std::wstring g_path;
static std::auto_ptr<ModuleManagerObjects> g_globals;

static WINDBG_EXTENSION_APIS ExtensionApis = {0};
static EXT_API_VERSION g_ExtApiVersion = 
    {
         5,
         5,
         EXT_API_VERSION_NUMBER64,
         0
    };

EXT_EXPORT 
LPEXT_API_VERSION WDBGAPI ExtensionApiVersion (void)
{
    return &g_ExtApiVersion;
}


// WinDbgExtensionDllInit
EXT_EXPORT
VOID WDBGAPI WinDbgExtensionDllInit (PWINDBG_EXTENSION_APIS lpExtensionApis, USHORT usMajorVersion, USHORT usMinorVersion)
{
     ExtensionApis = *lpExtensionApis;
}

namespace orthia
{

static void SetupPath(const std::wstring & path)
{
    if (path.empty())
    {
        std::stringstream res;
        res<<"Orthia path not found, setup "<<GLOBAL_ORTHIA_PATH_ANSI<<" or use !setup_path command";
        throw std::runtime_error(res.str());
    }
    if (!g_globals.get())
    {
        g_globals.reset(new ModuleManagerObjects);
    }
    g_globals->moduleManager.Reinit(path);
    g_path = path;
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
        SetupPath(g_path);
        return &g_globals->moduleManager;
    }
    
    if (pGlobal->moduleManager.GetPath() != g_path)
    {
        SetupPath(g_path);
    }
    return &pGlobal->moduleManager;
}
static void PrintUsage()
{
    dprintf("!help - display this text\n");
    dprintf("!setup_path <path> - init path\n");
    dprintf("!lm - displays module list\n");
    dprintf("!reload [/u] <module_address> - reloads module\n");
    dprintf("!x <address> - prints xrefs\n");
    dprintf("!t - trace (into)\n");
    dprintf("!p - step (over)\n");
}

void InitLib()
{
    try
    {
        std::wstring path = orthia::ExpandVariable(orthia::Trim(orthia::ToString(GLOBAL_ORTHIA_PATH_ANSI)));
        SetupPath(path);
    }
    catch(const std::exception & e)
    {
        &e;
        // do not care;
    }
}

}
// !help
DECLARE_API (help)
{
    dprintf("Orthia interface:\n\n");
    orthia::PrintUsage();
}

DECLARE_API (setup_path)
{
    ORTHIA_TRY
    std::wstring path = orthia::ExpandVariable(orthia::ToString(orthia::Trim(args)));
    orthia::SetupPath(path);
    ORTHIA_CATCH
}

DECLARE_API (lm)
{
    ORTHIA_TRY
        
    orthia::CModuleManager * pModuleManager = orthia::QueryModuleManager();
    std::vector<orthia::CModuleManager::ModuleInfo> modules;
    pModuleManager->QueryLoadedModules(&modules);
    for(std::vector<orthia::CModuleManager::ModuleInfo>::iterator it = modules.begin(), it_end = modules.end();
        it != it_end;
        ++it)
    {
        dprintf("%I64lx %s\n", it->address, it->name.c_str());
    }   

    ORTHIA_CATCH
}

DECLARE_API (reload)
{
    ORTHIA_TRY
        
    std::vector<std::wstring> words;
    orthia::Split(orthia::ToString(args), &words);
    bool bUnload = false;
    orthia::Address_type offset = 0;
    bool offsetInited = false;
    for(std::vector<std::wstring>::iterator it = words.begin(), it_end = words.end();
        it != it_end;
        ++it)
    {
        if (*it== L"/u")
        {
            bUnload = true;
            continue;
        }
        if (offsetInited)
            throw std::runtime_error("Unexpected argument: " + orthia::ToAnsiString_Silent(*it));
        PCSTR tail = 0;
        std::string strOffset = orthia::ToAnsiString_Silent(*it);
        GetExpressionEx(strOffsetc_str(), &offset, &tail);
        offsetInited = true;
    }
    
    orthia::CModuleManager * pModuleManager = orthia::QueryModuleManager();
    if (bUnload)
    {
        pModuleManager->UnloadModule(offset);
        return;
    }
    pModuleManager->ReloadModule(offset, orthia::QueryReader());

    ORTHIA_CATCH
}

DECLARE_API (x)
{
    ORTHIA_TRY

    // read parameters
    ULONG64 address = 0;
    
    if (!GetExpressionEx(args, &address, &args)) 
    {
        throw std::runtime_error("Address expression expected");
    }

    orthia::CModuleManager * pModuleManager = orthia::QueryModuleManager();

    std::vector<orthia::CModuleManager::ReferenceInfo> references;
    pModuleManager->QueryReferences(address, &references);

    for(std::vector<orthia::CModuleManager::ReferenceInfo>::iterator it = references.begin(), it_end = references.end();
        it != it_end;
        ++it)
    {
        dprintf("%I64lx\n", it->address);
    }   
    ORTHIA_CATCH
}

