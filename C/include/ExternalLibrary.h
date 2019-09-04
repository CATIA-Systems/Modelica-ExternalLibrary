#pragma once

#include "ModelicaUtilityFunctions.h"

#ifndef EXPORT
#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT const char* externalFunction(const char *filename, const char *moduleName, const char *functionName, const char *pythonHome, int nu, const double u[], int ny, double y[]);

EXPORT void* createExternalObject(const char *filename, const char *moduleName, const char *className, const char *pythonHome, const ModelicaUtilityFunctions_t *callbacks);

EXPORT void evaluateExternalObject(void *externalObject, int nu, const double u[], int ny, double y[]);

EXPORT void freeExternalObject(void *externalObject);

#ifdef __cplusplus
}
#endif
