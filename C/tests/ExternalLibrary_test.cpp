#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#define EXPORT typedef

#include "ExternalLibrary.h"

using namespace Catch::Matchers;


template<typename T> T *get(HMODULE libraryHandle, const char *functionName) {

# ifdef _WIN32
	auto *fp = GetProcAddress(libraryHandle, functionName);
# else
	auto *fp = dlsym(m_libraryHandle, functionName);
# endif

	REQUIRE(fp);

	return reinterpret_cast<T *>(fp);
}

static const char *s_errorMessage = nullptr;

static void ModelicaError(const char *string) {
	s_errorMessage = _strdup(string);
}


TEST_CASE("External functions can be loaded and called", "[ExternalLibrary]") {

	// load the shared library
# ifdef _WIN32
	auto l = LoadLibraryA("ExternalLibrary.dll");
# else
	auto l = dlopen("ExternalLibrary.so", RTLD_LAZY);
# endif

	ModelicaUtilityFunctions_t callbacks = { nullptr };

	callbacks.ModelicaError = ModelicaError;

	REQUIRE(l != nullptr);

	SECTION("Use stateless function") {

		auto fp_externalFunction = get<externalFunction>(l, "externalFunction");

		double u[] = { 1, 2 };
		double y[] = { 0, 0 };

		auto message = fp_externalFunction(DATA_FILE, PYTHON_HOME, 2, u, 2, y);

		REQUIRE(y[0] == 4);
		REQUIRE(y[1] == 5);
	}

	SECTION("Return an error message") {

		auto fp_createExternalObject = get<createExternalObject>(l, "createExternalObject");

		fp_createExternalObject(nullptr, nullptr, &callbacks);

		CHECK_THAT(s_errorMessage, Equals("Argument filename must not be NULL."));
	}

	SECTION("Use external object") {

		auto fp_createExternalObject   = get<createExternalObject>(l, "createExternalObject");
		auto fp_evaluateExternalObject = get<evaluateExternalObject>(l, "evaluateExternalObject");
		auto fp_freeExternalObject     = get<freeExternalObject>(l, "freeExternalObject");

		double u[] = { 1, 2 };
		double y[] = { 0, 0 };

		void *externalObject = fp_createExternalObject(DATA_FILE, PYTHON_HOME, &callbacks);

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