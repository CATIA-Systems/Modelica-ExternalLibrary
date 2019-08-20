#include <stdlib.h>
#include <fstream>

#include "ExternalLibrary.h"

#define UNUSED(x) (void)(x);


const char *externalFunction(const char *filename, const char *pythonHome, int nu, const double u[], int ny, double y[]) {

	UNUSED(pythonHome)

	std::ifstream infile(filename);

	if (!infile.is_open()) {
		return "Failed to open data file.";
	}

	double k;

	infile >> k;

	int s = (nu < ny) ? nu : ny;

	for (int i = 0; i < s; i++) {
		y[i] = u[i] + k;
	}

	return "";
}

class JetEngine {

public:
	JetEngine(double k) {
		this->k = k;
	}

	void evaluate(int nu, const double u[], int ny, double y[]) {

		int s = (nu < ny) ? nu : ny;

		for (int i = 0; i < s; i++) {
			y[i] = u[i] + k;
		}
	}

protected:
	double k;

};

void* createExternalObject(const char *filename, const char *pythonHome, const ModelicaUtilityFunctions_t *callbacks) {

	UNUSED(pythonHome)

	if (!filename) {
		callbacks->ModelicaError("Argument filename must not be NULL.");
		return nullptr;
	}

	std::ifstream infile(filename);

	if (!infile.is_open()) {
		callbacks->ModelicaError("Failed to open data file.");
		return nullptr;
	}

	double k;

	infile >> k;
	
	return new JetEngine(k);
}

void evaluateExternalObject(void *externalObject, int nu, const double u[], int ny, double y[]) {
	if (!externalObject) return;
	JetEngine *jetEngine = reinterpret_cast<JetEngine*>(externalObject);
	jetEngine->evaluate(nu, u, ny, y);
}

void freeExternalObject(void *externalObject) {
	if (!externalObject) return;
	JetEngine *jetEngine = reinterpret_cast<JetEngine*>(externalObject);
	delete jetEngine;
}
