#ifndef DIANA_ANALYZE2_H
#define DIANA_ANALYZE2_H

#include "diana_core.h"

struct _dianaParserResult;
struct _dianaContext;
int Diana_AnalyzeInstructionStackDiff(struct _dianaContext * pContext,
                                      struct _dianaParserResult * pResult,
                                      OPERAND_SIZE_SIGNED * pESPDiff,
                                      int * pIsCall);

#endif