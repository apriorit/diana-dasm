#ifndef DIANA_CONFIG_H
#define DIANA_CONFIG_H


#ifdef _M_IX86

#define DIANA_CFG_USE_INLINE_ASSEMBLER  

#endif


#if defined(_M_IX86)

#define DIANA_CFG_I386

#endif


#define DIANA_PROCESSOR_USE_SOFTFLOAT_FPU


#ifndef DIANA_INLINE_C
# if __GNUC__ && !__GNUC_STDC_INLINE__
#  define DIANA_INLINE_C extern inline
# else
#  define DIANA_INLINE_C __forceinline
# endif
#endif

#ifndef DIANA_CONFIGURATION_FILE
#define DIANA_CONFIGURATION_FILE "./diana_custom_config.inc"
#endif

#ifdef DIANA_CONFIGURATION_FILE
#include DIANA_CONFIGURATION_FILE
#endif 


#include "diana_apply_config_rules.h"

#endif