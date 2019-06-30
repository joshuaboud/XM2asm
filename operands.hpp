#ifndef OPERANDS_H
#define OPERANDS_H

#include <string>

#define CONST_CNT 8

#define CEX_FLAG_CNT 17
typedef struct{
	std::string flag;
	int encoding;
} CEX_Flag;
extern CEX_Flag cexFlags[CEX_FLAG_CNT];

#endif
