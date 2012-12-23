#ifndef DIANA_CONFIG_H
#define DIANA_CONFIG_H

#ifdef _M_IX86

#define DIANA_CFG_USE_INLINE_ASSEMBLER  

#endif


#if defined(_M_IX86) // || defined(__i386__) - to be implemented

#define DIANA_CFG_I386

#endif


#endif