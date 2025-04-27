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

EXPORT const char* externalFunction(
	const char* filename,
	int nu,
	const double u[],
	int ny,
	const char* pythonDllPath,
	const char* pythonExePath,
	const char* moduleName,
	const char* functionName,
	double y[]
);

EXPORT void* createExternalObject(
	const char* filename,
	const ModelicaUtilityFunctions_t* callbacks,
	const char* pythonDllPath,
	const char* pythonExePath,
	const char* moduleName,
	const char* className
);

EXPORT void evaluateExternalObject(void *externalObject, int nu, const double u[], int ny, double y[]);

EXPORT void freeExternalObject(void *externalObject);

#ifdef __cplusplus
}
#endif
