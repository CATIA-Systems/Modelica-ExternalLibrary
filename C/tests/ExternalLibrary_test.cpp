#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch_amalgamated.hpp"


# ifdef _WIN32
#include <Windows.h>
# else
#include <unistd.h>
#include <dlfcn.h>
typedef void * HMODULE;
#include <stdio.h>
#include <limits.h>
#include <libgen.h>
# endif

#ifdef _MSC_VER
#define strdup _strdup
#endif

#define EXPORT typedef

#include "ExternalLibrary.h"

using namespace Catch::Matchers;


#define MODULE_NAME   "external_library"
#define FUNCTION_NAME "external_library_function"
#define CLASS_NAME    "ExternalLibraryObject"


template<typename T> T *get(HMODULE libraryHandle, const char *functionName) {

# ifdef _WIN32
	auto *fp = GetProcAddress(libraryHandle, functionName);
# else
	auto *fp = dlsym(libraryHandle, functionName);
# endif

	REQUIRE(fp);

	return reinterpret_cast<T *>(fp);
}

static const char *s_errorMessage = nullptr;

static void ModelicaError(const char *string) {
	s_errorMessage = strdup(string);
}


TEST_CASE("External functions can be loaded and called", "[ExternalLibrary]") {

	// load the shared library
# ifdef _WIN32
	auto l = LoadLibraryA("ExternalLibrary.dll");
# else
	char buf[FILENAME_MAX];
	ssize_t nbytes, bufsiz;
	// get the path of the executable
	nbytes = readlink("/proc/self/exe", buf, FILENAME_MAX);
	auto basename = dirname(buf);
	auto filename = strcat(basename, "/ExternalLibrary.so");
	auto l = dlopen(filename, RTLD_NOW);
# endif

	ModelicaUtilityFunctions_t callbacks = { nullptr };

	callbacks.ModelicaError = ModelicaError;

	REQUIRE(l != nullptr);

	SECTION("Use stateless function") {

		auto fp_externalFunction = get<externalFunction>(l, "externalFunction");

		double u[] = { 1, 2 };
		double y[] = { 0, 0 };

		auto message = fp_externalFunction(DATA_FILE, 2, u, 2, PYTHON_DLL, /*PYTHON_EXE*/"", MODULE_NAME, FUNCTION_NAME, y);

		CHECK_THAT(message, Equals(""));
		CHECK(y[0] == 4);
		CHECK(y[1] == 5);
	}

	SECTION("Return an error message") {

		auto fp_createExternalObject = get<createExternalObject>(l, "createExternalObject");

		fp_createExternalObject(nullptr, &callbacks, nullptr, nullptr, nullptr, nullptr);

		CHECK_THAT(s_errorMessage, Equals("Argument filename must not be NULL."));
	}

	SECTION("Use external object") {

		auto fp_createExternalObject   = get<createExternalObject>(l, "createExternalObject");
		auto fp_evaluateExternalObject = get<evaluateExternalObject>(l, "evaluateExternalObject");
		auto fp_freeExternalObject     = get<freeExternalObject>(l, "freeExternalObject");

		double u[] = { 1, 2 };
		double y[] = { 0, 0 };

		void *externalObject = fp_createExternalObject(DATA_FILE, &callbacks, PYTHON_DLL, PYTHON_EXE, MODULE_NAME, CLASS_NAME);

		REQUIRE(externalObject);

		fp_evaluateExternalObject(externalObject, 2, u, 2, y);

		REQUIRE(y[0] == 4);
		REQUIRE(y[1] == 5);

		fp_freeExternalObject(externalObject);
	}

	// free the shared library
# ifdef _WIN32
	FreeLibrary(l);
# else
	dlclose(l);
# endif

}