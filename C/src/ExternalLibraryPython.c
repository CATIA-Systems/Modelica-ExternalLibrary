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

#define PYTHON_MODULE "external_library"

const char* externalFunction(const char* filename, const char* moduleName, const char* functionName, const char* pythonHome, int nu, const double u[], int ny, double y[]) {

#ifdef _WIN32
	DWORD dwAttrib = GetFileAttributesA(pythonHome);

	int valid = dwAttrib != INVALID_FILE_ATTRIBUTES;
	int isdir = dwAttrib & FILE_ATTRIBUTE_DIRECTORY;

	if (!pythonHome || !valid || !isdir) {
		return "Argument pythonHome must be a valid directory.";
	}

	BOOL res = SetDllDirectoryA(pythonHome);
#endif

	size_t size;
	const wchar_t *python_home_w = Py_DecodeLocale(pythonHome, &size);

	Py_SetPythonHome(python_home_w);

	Py_Initialize();

	PyObject *py_moduleName = PyUnicode_FromString(PYTHON_MODULE);
	
	PyObject *py_module = PyImport_Import(py_moduleName);
	
	if (!py_module) {
		return "Failed to load Python module \"" PYTHON_MODULE "\".";
	}

	PyObject *py_function = PyObject_GetAttrString(py_module, "external_library_function");

	if (!py_function) {
		return "Failed to load function \"external_library_function\" from Python module \"" PYTHON_MODULE "\".";
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


void* createExternalObject(const char* filename, const char* moduleName, const char* className, const char* pythonHome, const ModelicaUtilityFunctions_t* callbacks) {

	if (!filename) {
		callbacks->ModelicaError("Argument filename must not be NULL.");
		return NULL;
	}

#ifdef _WIN32
	DWORD dwAttrib = GetFileAttributesA(pythonHome);

	int valid = dwAttrib != INVALID_FILE_ATTRIBUTES;
	int isdir = dwAttrib & FILE_ATTRIBUTE_DIRECTORY;

	if (!pythonHome || !valid || !isdir) {
		callbacks->ModelicaError("Argument pythonHome must be a valid directory.");
		return NULL;
	}

	BOOL res = SetDllDirectoryA(pythonHome);
#endif

	size_t size;
	const wchar_t *python_home_w = Py_DecodeLocale(pythonHome, &size);

	Py_SetPythonHome(python_home_w);

	Py_Initialize();

	PythonObjects *o = malloc(sizeof(PythonObjects));

	if (!o) {
		return NULL;
	}

	o->moduleName = PyUnicode_FromString(PYTHON_MODULE);
	o->module = PyImport_Import(o->moduleName);
	o->module = PyImport_ReloadModule(o->module);

	if (!o->module) {
		return NULL;
	}

	o->methodName = PyUnicode_DecodeFSDefault("evaluate");

	o->class = PyObject_GetAttrString(o->module, "ExternalLibraryObject");

	//int is_type = PyType_Check(o->class);

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
