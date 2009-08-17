#ifndef DIANA_TABLES_32_H
#define DIANA_TABLES_32_H

#include "diana_core.h"

int Diana_ReadIndexStructure32(DI_CHAR iOpSize,
                               unsigned char postByte,
                               DianaReadStream * pStream, 
                               DianaOperandValue * pValue,
                               DianaValueType * pType);

#endif