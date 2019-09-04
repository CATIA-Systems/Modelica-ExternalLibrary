#include <stdlib.h>

#include <Windows.h>

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#include "ExternalLibrary.h"

#define PYTHON_MODULE "external_library"
#define ENV_VAR_MAX_SIZE 32767

ModelicaUtilityFunctions_t s_callbacks = { NULL };


static const char *initializePython(const char *pythonHome) {

#ifdef _WIN32
	DWORD dwAttrib = GetFileAttributes(pythonHome);

	int valid = dwAttrib != INVALID_FILE_ATTRIBUTES;
	int isdir = dwAttrib & FILE_ATTRIBUTE_DIRECTORY;

	if (!pythonHome || !valid || !isdir) {
		return "Argument pythonHome must be a valid directory.";
	}

	char path[ENV_VAR_MAX_SIZE];

	strcpy(path, pythonHome);
	strcat(path, ";");
	strcat(path, pythonHome);
	strcat(path, "\\Library\\mingw-w64\\bin;");
	strcat(path, pythonHome);
	strcat(path, "\\Library\\usr\\bin;");
	strcat(path, pythonHome);
	strcat(path, "\\Library\\bin;");
	strcat(path, pythonHome);
	strcat(path, "\\Library\\Scripts;");
	strcat(path, pythonHome);
	strcat(path, "\\bin;");

	size_t len = strlen(path);

	// add the Anaconda environment to the system path 
	GetEnvironmentVariable("PATH", &path[len], ENV_VAR_MAX_SIZE);
	SetEnvironmentVariable("PATH", path);
#endif

	size_t size;
	const wchar_t *python_home_w = Py_DecodeLocale(pythonHome, &size);

	Py_SetPythonHome(python_home_w);

	Py_Initialize();

	return NULL;
}

static const char *handlePythonError() {

	PyObject *err = PyErr_Occurred();

	if (!err) return NULL;

	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);

	PyObject *pystr = PyObject_Str(pvalue);

	Py_ssize_t size;
	const char *str = PyUnicode_AsUTF8AndSize(pystr, &size);

	return str;
}

#define HANDLE_PYTHON_ERROR if (handlePythonError()) { goto out; }

#define HANDLE_ERROR(F) if (error = F) { goto out; }

const char *externalFunction(const char *filename, const char *moduleName, const char *functionName, const char *pythonHome, int nu, const double u[], int ny, double y[]) {

	const char *error = NULL;
	
	HANDLE_ERROR(initializePython(pythonHome))

	PyObject *py_moduleName = PyUnicode_FromString(moduleName);
	
	PyObject *py_module = PyImport_Import(py_moduleName);
	HANDLE_PYTHON_ERROR

	PyObject *py_function = PyObject_GetAttrString(py_module, functionName);
	HANDLE_PYTHON_ERROR

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
		error = "The Python function returned the wrong number of values.";
		goto out;
	}

	for (int i = 0; i < ny; i++) {
		PyObject *py_v = PyTuple_GetItem(py_y, i);
		y[i] = PyFloat_AsDouble(py_v);
	}

	Py_FinalizeEx();

out:
	return error ? error : "";
}

typedef struct {
	PyObject *moduleName;
	PyObject *module;
	PyObject *class;
	PyObject *instance;
	PyObject *methodName;
} PythonObjects;

void* createExternalObject(const char *filename, const char *moduleName, const char *className, const char *pythonHome, const ModelicaUtilityFunctions_t *callbacks) {

	s_callbacks = *callbacks;
	const char *error = NULL;

	if (!filename) {
		error = "Argument filename must not be NULL.";
		goto out;
	}

	if (!moduleName) {
		error = "Argument moduleName must not be NULL.";
		goto out;
	}

	if (!className) {
		error = "Argument className must not be NULL.";
		goto out;
	}

	HANDLE_ERROR(initializePython(pythonHome))

	PythonObjects o;

	o.moduleName = PyUnicode_FromString(moduleName);
	o.module = PyImport_Import(o.moduleName);
	HANDLE_PYTHON_ERROR

	o.methodName = PyUnicode_DecodeFSDefault("evaluate");

	o.class = PyObject_GetAttrString(o.module, className);
	HANDLE_PYTHON_ERROR

	PyObject *py_filename = PyUnicode_FromString(filename);

	PyObject *py_ctor_args = PyTuple_New(1);

	PyTuple_SetItem(py_ctor_args, 0, py_filename);

	o.instance = PyObject_CallObject(o.class, py_ctor_args);
	HANDLE_PYTHON_ERROR

out:
	if (error) {
		if (s_callbacks.ModelicaError) {
			s_callbacks.ModelicaError(error);
		}
		return NULL;
	}

	PythonObjects *po = malloc(sizeof(PythonObjects));

	*po = o;

	return po;
}

void evaluateExternalObject(void *externalObject, int nu, const double u[], int ny, double y[]) {

	const char *error = NULL;

	if (!externalObject) {
		error = "Argument externalObject must not be NULL.";
		goto out;
	}

	PythonObjects *o = (PythonObjects *)externalObject;

	PyObject *py_u = PyTuple_New(nu);

	for (int i = 0; i < nu; i++) {
		PyObject *py_v = PyFloat_FromDouble(u[i]);
		PyTuple_SetItem(py_u, i, py_v);
	}

	PyObject *py_args = PyTuple_New(1);
	PyTuple_SetItem(py_args, 0, py_u);

	PyObject *py_y = PyObject_CallMethodObjArgs(o->instance, o->methodName, py_u, NULL);
	HANDLE_PYTHON_ERROR

	Py_ssize_t size = PyTuple_Size(py_y);

	if (ny != size) return;

	for (int i = 0; i < ny; i++) {
		PyObject *py_v = PyTuple_GetItem(py_y, i);
		y[i] = PyFloat_AsDouble(py_v);
	}

out:
	if (error) {
		if (s_callbacks.ModelicaError) {
			s_callbacks.ModelicaError(error);
		}
		return NULL;
	}
}

void freeExternalObject(void *externalObject) {
	PythonObjects *o = (PythonObjects *)externalObject;
	if (o) free(o);
}
