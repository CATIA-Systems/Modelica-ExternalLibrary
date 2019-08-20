#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

	void  (*ModelicaMessage)(const char *string);
	void  (*ModelicaFormatMessage)(const char *string, ...);
	void  (*ModelicaVFormatMessage)(const char *string, va_list);
	void  (*ModelicaError)(const char *string);
	void  (*ModelicaFormatError)(const char *string, ...);
	void  (*ModelicaVFormatError)(const char *string, va_list);
	char* (*ModelicaAllocateString)(size_t len);
	char* (*ModelicaAllocateStringWithErrorReturn)(size_t len);

} ModelicaUtilityFunctions_t;

#ifdef __cplusplus
}
#endif
