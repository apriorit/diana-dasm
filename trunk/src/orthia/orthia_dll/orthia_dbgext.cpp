#define _CRT_SECURE_NO_WARNINGS
#include "orthia_dbgext.h"
#include "algorithm"
#undef min

WINDBG_EXTENSION_APIS   ExtensionApis = {};
static PDEBUG_CLIENT4        g_ExtClient = 0;
static PDEBUG_CONTROL        g_ExtControl = 0;
static PDEBUG_SYMBOLS2       g_ExtSymbols = 0;
static ULONG   g_TargetMachine = 0;
static BOOL    g_Connected = 0;


void
ExtRelease(void)
{
    g_ExtClient = NULL;
    EXT_RELEASE(g_ExtControl);
    EXT_RELEASE(g_ExtSymbols);
}

HRESULT
ExtQuery(PDEBUG_CLIENT4 Client)
{
    HRESULT Status;

    if ((Status = Client->QueryInterface(__uuidof(IDebugControl),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols2),
                                (void **)&g_ExtSymbols)) != S_OK)
    {
    goto Fail;
    }
    g_ExtClient = Client;

    return S_OK;

 Fail:
    ExtRelease();
    return Status;
}


extern "C"
HRESULT
CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    IDebugClient *DebugClient;
    PDEBUG_CONTROL DebugControl;
    HRESULT Hr;

    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;
    Hr = S_OK;

    if ((Hr = DebugCreate(__uuidof(IDebugClient),
                          (void **)&DebugClient)) != S_OK)
    {
        return Hr;
    }

    if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                  (void **)&DebugControl)) == S_OK)
    {

        //
        // Get the windbg-style extension APIS
        //
        ExtensionApis.nSize = sizeof (ExtensionApis);
        Hr = DebugControl->GetWindbgExtensionApis64(&ExtensionApis);

        DebugControl->Release();

    }
    DebugClient->Release();
    return Hr;
}

extern "C"
void
CALLBACK
DebugExtensionUninitialize(void)
{
    return;
}

extern "C"
void
CALLBACK
DebugExtensionNotify(ULONG Notify, ULONG64 Argument)
{
    UNREFERENCED_PARAMETER(Argument);

    //
    // The first time we actually connect to a target
    //

    if ((Notify == DEBUG_NOTIFY_SESSION_ACCESSIBLE) && (!g_Connected))
    {
        IDebugClient *DebugClient;
        HRESULT Hr;
        PDEBUG_CONTROL DebugControl;

        if ((Hr = DebugCreate(__uuidof(IDebugClient),
                              (void **)&DebugClient)) == S_OK)
        {
            //
            // Get the architecture type.
            //

            if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                       (void **)&DebugControl)) == S_OK)
            {
                if ((Hr = DebugControl->GetActualProcessorType(
                                             &g_TargetMachine)) == S_OK)
                {
                    g_Connected = TRUE;
                }
                //NotifyOnTargetAccessible(DebugControl);
                DebugControl->Release();
            }

            DebugClient->Release();
        }
    }


    if (Notify == DEBUG_NOTIFY_SESSION_INACTIVE)
    {
        g_Connected = FALSE;
        g_TargetMachine = 0;
    }

    return;
}

//IMAGE_FILE_MACHINE_I386
//IMAGE_FILE_MACHINE_AMD64
ULONG DbgExt_GetTargetMachine()
{
    if (!g_Connected) 
    {
        return 0;
    }
    return g_TargetMachine;
}

bool DbgExt_GetNameByOffset(ULONG64 offset,
                            PSTR nameBuffer,
                            ULONG nameBufferSize,
                            PULONG pResultNameSize,
                            ULONG64 * pDisplacement)
{
    *pDisplacement = 0;
    *pResultNameSize = 0;
    if (!SUCCEEDED(g_ExtSymbols->GetNameByOffset(offset, 
                                                    nameBuffer, 
                                                    nameBufferSize, 
                                                    pResultNameSize, 
                                                    pDisplacement)))
    {
        char defaultValue[] = "<unknown>";
        strncat(nameBuffer, defaultValue, std::min<ULONG>(sizeof(defaultValue)/sizeof(defaultValue[0])-1, nameBufferSize));
        return false;
    }
    return true;
}