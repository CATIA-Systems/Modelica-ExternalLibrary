#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#include "ExternalLibrary.h"


static const char* initializePython(
	const char* pythonDllPath, 
	const char* pythonExePath
) {

	if (!pythonDllPath || strlen(pythonDllPath) == 0) {
		return "Argument filename must not be NULL or empty.";
	}

	HMODULE pythonDLL = LoadLibraryA(pythonDllPath);

	if (!pythonDLL) {
		return "Failed to load Python DLL.";
	}

	PyConfig config;
	PyStatus status;

	if (pythonExePath && strlen(pythonExePath) > 0) {

		PyConfig_InitIsolatedConfig(&config);

		const size_t cSize = strlen(pythonExePath) + 1;
		wchar_t* venv_executable = calloc(cSize, sizeof(wchar_t));
		mbstowcs(venv_executable, pythonExePath, cSize);

		PyConfig_SetString(&config, &config.executable, venv_executable);

		free(venv_executable);

	} else {

		PyConfig_InitPythonConfig(&config);

	}
 
	status = Py_InitializeFromConfig(&config);

	if (PyStatus_Exception(status)) {
		Py_ExitStatusException(status);
	}

	return NULL;
}


const char* externalFunction(
	const char* filename,
	int nu,
	const double u[],
	int ny,
	const char* pythonDllPath,
	const char* pythonExePath,
	const char* moduleName,
	const char* functionName,
	double y[]
) {

	if (!filename) {
		return "Argument filename must not be NULL.";
	}

	if (!moduleName) {
		return "Argument moduleName must not be NULL.";
	}

	const char* initError = initializePython(pythonDllPath, pythonExePath);

	if (initError) {
		return initError;
	}

	PyObject *py_moduleName = PyUnicode_FromString(moduleName);
	
	PyObject *py_module = PyImport_Import(py_moduleName);
	
	if (!py_module) {
		return "Failed to load Python module.";
	}

	PyObject *py_function = PyObject_GetAttrString(py_module, functionName);

	if (!py_function) {
		return "Failed to load function from Python module.";
	}

	PyObject *py_filename = PyUnicode_FromString(filename);

	PyObject *py_u = PyTuple_New(nu);

	for (int i = 0; i < nu; i++) {
		PyObject *py_v = PyFloat_FromDouble(u[i]);
		PyTuple_SetItem(py_u, i, py_v);
	}

	PyObject *py_args = PyTuple_New(2);
	PyTuple_SetItem(py_args, 0, py_filename);
	PyTuple_SetItem(py_args, 1, py_u);

	PyObject *py_y = PyObject_CallObject(py_function, py_args);

	Py_ssize_t py_size = PyTuple_Size(py_y);

	if (ny != py_size) {
		return "Function \"interpolate\" returned the wrong number of values.";
	}

	for (int i = 0; i < ny; i++) {
		PyObject *py_v = PyTuple_GetItem(py_y, i);
		y[i] = PyFloat_AsDouble(py_v);
	}

	Py_FinalizeEx();

	return "";
}

typedef struct {
	PyObject *moduleName;
	PyObject *module;
	PyObject *class;
	PyObject *instance;
	PyObject *methodName;
} PythonObjects;


void* createExternalObject(
	const char* filename,
	const ModelicaUtilityFunctions_t* callbacks,
	const char* pythonDllPath,
	const char* pythonExePath,
	const char* moduleName,
	const char* className
) {

	if (!filename) {
		callbacks->ModelicaError("Argument filename must not be NULL.");
		return NULL;
	}

	if (!moduleName) {
		callbacks->ModelicaError("Argument moduleName must not be NULL.");
		return NULL;
	}

	if (!className) {
		callbacks->ModelicaError("Argument className must not be NULL.");
		return NULL;
	}

	const char* initError = initializePython(pythonDllPath, pythonExePath);

	if (initError) {
		return initError;
	}

	PythonObjects *o = malloc(sizeof(PythonObjects));

	if (!o) {
		return NULL;
	}

	o->moduleName = PyUnicode_FromString(moduleName);

	o->module = PyImport_Import(o->moduleName);

	if (!o->module) {
		callbacks->ModelicaFormatError("Failed to import module %s.", moduleName);
		return NULL;
	}
	
	o->module = PyImport_ReloadModule(o->module);

	if (!o->module) {
		callbacks->ModelicaFormatError("Failed to reload module %s.", moduleName);
		return NULL;
	}

	o->methodName = PyUnicode_DecodeFSDefault("evaluate");

	o->class = PyObject_GetAttrString(o->module, className);

	if (!o->class) {
		callbacks->ModelicaFormatError("Failed to load class %s from module %s.", className, moduleName);
		return NULL;
	}

	PyObject *py_filename = PyUnicode_FromString(filename);

	PyObject *py_ctor_args = PyTuple_New(1);

	PyTuple_SetItem(py_ctor_args, 0, py_filename);

	o->instance = PyObject_CallObject(o->class, py_ctor_args);

	PyErr_Print();

	return o;
}

void evaluateExternalObject(void *externalObject, int nu, const double u[], int ny, double y[]) {

	PythonObjects *o = (PythonObjects *)externalObject;

	PyObject *py_u = PyTuple_New(nu);

	for (int i = 0; i < nu; i++) {
		PyObject *py_v = PyFloat_FromDouble(u[i]);
		PyTuple_SetItem(py_u, i, py_v);
	}

	PyObject *py_args = PyTuple_New(1);
	PyTuple_SetItem(py_args, 0, py_u);

	PyObject *py_y = PyObject_CallMethodObjArgs(o->instance, o->methodName, py_u, NULL);

	PyErr_Print();

	Py_ssize_t size = PyTuple_Size(py_y);

	if (ny != size) return;

	for (int i = 0; i < ny; i++) {
		PyObject *py_v = PyTuple_GetItem(py_y, i);
		y[i] = PyFloat_AsDouble(py_v);
	}
}

void freeExternalObject(void *externalObject) {
	PythonObjects *o = (PythonObjects *)externalObject;
	if (o) free(o);
}
