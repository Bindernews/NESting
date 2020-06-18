/* ------------------------------------------------------------
name: "BlockTriangle"
Code generated with Faust 2.20.2 (https://faust.grame.fr)
Compilation options: -lang cpp -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __Triangle_H__
#define  __Triangle_H__


#define FAUSTFLOAT iplug::sample

#include "IPlugConstants.h"
#include "IPlugFaust.h"

/* BEGIN AUTO GENERATED BY THE FAUST COMPILER ... */
//#define max std::max
//#define min std::min
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>

const static float fTriangleSIG0Wave0[512] = {8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f};
class TriangleSIG0 {
	
  private:
	
	int fTriangleSIG0Wave0_idx;
	
  public:
	
	int getNumInputsTriangleSIG0() {
		return 0;
	}
	int getNumOutputsTriangleSIG0() {
		return 1;
	}
	int getInputRateTriangleSIG0(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRateTriangleSIG0(int channel) {
		int rate;
		switch ((channel)) {
			case 0: {
				rate = 0;
				break;
			}
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	
	void instanceInitTriangleSIG0(int sample_rate) {
		fTriangleSIG0Wave0_idx = 0;
	}
	
	void fillTriangleSIG0(int count, float* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			table[i] = fTriangleSIG0Wave0[fTriangleSIG0Wave0_idx];
			fTriangleSIG0Wave0_idx = ((1 + fTriangleSIG0Wave0_idx) % 512);
		}
	}

};

static TriangleSIG0* newTriangleSIG0() { return (TriangleSIG0*)new TriangleSIG0(); }
static void deleteTriangleSIG0(TriangleSIG0* dsp) { delete dsp; }

static float ftbl0TriangleSIG0[512];

#ifndef FAUSTCLASS 
#define FAUSTCLASS Triangle
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

class Triangle : public dsp {
	
 private:
	
	FAUSTFLOAT fHslider0;
	int fSampleRate;
	float fConst0;
	FAUSTFLOAT fHslider1;
	float fRec0[2];
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("filename", "BlockTriangle.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.1");
		m->declare("name", "BlockTriangle");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "0.0");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	virtual int getInputRate(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	virtual int getOutputRate(int channel) {
		int rate;
		switch ((channel)) {
			case 0: {
				rate = 1;
				break;
			}
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	
	static void classInit(int sample_rate) {
		TriangleSIG0* sig0 = newTriangleSIG0();
		sig0->instanceInitTriangleSIG0(sample_rate);
		sig0->fillTriangleSIG0(512, ftbl0TriangleSIG0);
		deleteTriangleSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = (1.0f / std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate))));
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(0.5f);
		fHslider1 = FAUSTFLOAT(440.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fRec0[l0] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual Triangle* clone() {
		return new Triangle();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openHorizontalBox("1");
		ui_interface->declare(&fHslider1, "unit", "Hz");
		ui_interface->addHorizontalSlider("freq", &fHslider1, 440.0f, 20.0f, 20000.0f, 1.0f);
		ui_interface->addHorizontalSlider("gain", &fHslider0, 0.5f, 0.0f, 1.0f, 0.00999999978f);
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = float(fHslider0);
		float fSlow1 = (fConst0 * float(fHslider1));
		for (int i = 0; (i < count); i = (i + 1)) {
			fRec0[0] = (fSlow1 + (fRec0[1] - std::floor((fSlow1 + fRec0[1]))));
			output0[i] = FAUSTFLOAT((fSlow0 * ((0.125f * ftbl0TriangleSIG0[int((512.0f * fRec0[0]))]) + -1.0f)));
			fRec0[1] = fRec0[0];
		}
	}

};
//#undef max
//#undef min
/* ... END AUTO GENERATED BY THE FAUST COMPILER  */
using namespace iplug;

class Faust_Triangle : public IPlugFaust
{
public:
	Faust_Triangle(const char* name, const char* inputDSPFile = 0, int nVoices = 1, int rate = 1,
						const char* outputCPPFile = 0, const char* drawPath = 0, const char* libraryPath = FAUST_LIBRARY_PATH)
	: IPlugFaust(name, nVoices)
	{
	}

	void Init() override
	{
		mDSP = std::make_unique<FAUSTCLASS>();
		mDSP->buildUserInterface(this);
		BuildParameterMap();
		mInitialized = true;
	}
};

#undef FAUSTCLASS

#endif
