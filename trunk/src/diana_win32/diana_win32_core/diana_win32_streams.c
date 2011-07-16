#include "diana_win32_streams.h"

static int DianaWin32RandomRead(void * pThis, 
                                    OPERAND_SIZE selector,
                                    OPERAND_SIZE offset,
                                    void * pBuffer, 
                                    OPERAND_SIZE iBufferSize, 
                                    OPERAND_SIZE * readed,
                                    int flags,
                                    DianaUnifiedRegister segReg)
{
    DI_INT16 dsValue = 0;
    {
    __asm mov dsValue, ds
    }
    if (dsValue == selector)
    {
        memcpy(pBuffer, (void*)offset, (size_t)iBufferSize);
        *readed = (size_t)iBufferSize;
        return DI_SUCCESS;
    }
        
    {
        DI_UINT32 offset32 = (DI_UINT32)offset;
        DI_UINT32 iBufferSize32 = (DI_UINT32)iBufferSize;
        DI_UINT16 selector16 = (DI_UINT16)selector;
    
        __asm push ds
        __asm push es
        __asm push edi
        __asm push esi
        __asm push ecx

        __asm mov si, ds
        __asm mov es, si
        __asm mov ds, selector16;
        __asm mov edi, pBuffer
        __asm mov esi, offset32
        __asm mov ecx, iBufferSize32
        __asm rep movsb

        __asm pop ecx
        __asm pop esi
        __asm pop edi
        __asm pop es
        __asm pop ds
    }
    *readed = (size_t)iBufferSize;
    return DI_SUCCESS;
}
static int DianaWin32RandomWrite(void * pThis, 
                                     OPERAND_SIZE selector,
                                     OPERAND_SIZE offset,
                                     void * pBuffer, 
                                     OPERAND_SIZE iBufferSize, 
                                     OPERAND_SIZE * wrote,
                                     int flags,
                                     DianaUnifiedRegister segReg)
{
    DI_INT16 dsValue = 0;
    {
    __asm mov dsValue, ds
    }
    if (dsValue == selector)
    {
        memcpy((void*)offset, pBuffer, (size_t)iBufferSize);
        *wrote = (size_t)iBufferSize;
        return DI_SUCCESS;
    }
        
    {
        DI_UINT32 offset32 = (DI_UINT32)offset;
        DI_UINT32 iBufferSize32 = (DI_UINT32)iBufferSize;
        DI_UINT16 selector16 = (DI_UINT16)selector;
    
        __asm push es
        __asm push edi
        __asm push esi
        __asm push ecx

        __asm mov es, selector16;
        __asm mov edi, offset32
        __asm mov esi, pBuffer
        __asm mov ecx, iBufferSize32
        __asm rep movsb

        __asm pop ecx
        __asm pop esi
        __asm pop edi
        __asm pop es
    }

    *wrote = (size_t)iBufferSize;
    return DI_SUCCESS;
}
void DianaWin32Stream_Init(DianaWin32Stream * pStream)
{
    pStream->m_parent.pReadFnc = DianaWin32RandomRead;
    pStream->m_parent.pWriteFnc = DianaWin32RandomWrite;
}


//------------ REMOTE
static int DianaWin32RandomWrite_Remote(void * pThis, 
                                        OPERAND_SIZE selector,
                                        OPERAND_SIZE offset,
                                        void * pBuffer, 
                                        OPERAND_SIZE iBufferSize, 
                                        OPERAND_SIZE * wrote,
                                        int flags,
                                        DianaUnifiedRegister segReg)
{
    DianaWin32RemoteStream * p = pThis;

    if (segReg != reg_DS && segReg != reg_CS && segReg != reg_SS)
        return DI_ERROR_NOT_USED_BY_CORE;


    {
        SIZE_T res = 0;
        if (!WriteProcessMemory(p->m_hProcess, 
                            (void*)offset, 
                            pBuffer, 
                            (SIZE_T)iBufferSize,
                            &res))
        {
            return DI_WIN32_ERROR;
        }
        *wrote = res;
    }
    return DI_SUCCESS;
}

static int DianaWin32RandomRead_Remote(void * pThis, 
                                       OPERAND_SIZE selector,
                                       OPERAND_SIZE offset,
                                       void * pBuffer, 
                                       OPERAND_SIZE iBufferSize, 
                                       OPERAND_SIZE * readed,
                                       int flags,
                                       DianaUnifiedRegister segReg)
{
    DianaWin32RemoteStream * p = pThis;
    
    if (segReg != reg_DS && segReg != reg_CS && segReg != reg_SS)
        return DI_ERROR_NOT_USED_BY_CORE;

    {
        SIZE_T res = 0;
        if (!ReadProcessMemory(p->m_hProcess, 
                            (void*)offset, 
                            pBuffer, 
                            (SIZE_T)iBufferSize,
                            &res))
        {
            return DI_WIN32_ERROR;
        }
        *readed = res;
    }
    return DI_SUCCESS;
}

void DianaWin32RemoteStream_Init(DianaWin32RemoteStream * pStream,
                                 HANDLE hProcess)
{
    pStream->m_parent.pReadFnc = DianaWin32RandomRead_Remote;
    pStream->m_parent.pWriteFnc = DianaWin32RandomWrite_Remote;
    pStream->m_hProcess = hProcess;
}
