/* ------------------------------------------------------------
name: "Triangle2A03"
Code generated with Faust 2.20.2 (https://faust.grame.fr)
Compilation options: -lang cpp -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __NEStingBlock_H__
#define  __NEStingBlock_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>

const static float fNEStingBlockSIG0Wave0[512] = {8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f};
class NEStingBlockSIG0 {
	
  private:
	
	int fNEStingBlockSIG0Wave0_idx;
	
  public:
	
	int getNumInputsNEStingBlockSIG0() {
		return 0;
	}
	int getNumOutputsNEStingBlockSIG0() {
		return 1;
	}
	int getInputRateNEStingBlockSIG0(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRateNEStingBlockSIG0(int channel) {
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
	
	void instanceInitNEStingBlockSIG0(int sample_rate) {
		fNEStingBlockSIG0Wave0_idx = 0;
	}
	
	void fillNEStingBlockSIG0(int count, float* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			table[i] = fNEStingBlockSIG0Wave0[fNEStingBlockSIG0Wave0_idx];
			fNEStingBlockSIG0Wave0_idx = ((1 + fNEStingBlockSIG0Wave0_idx) % 512);
		}
	}

};

static NEStingBlockSIG0* newNEStingBlockSIG0() { return (NEStingBlockSIG0*)new NEStingBlockSIG0(); }
static void deleteNEStingBlockSIG0(NEStingBlockSIG0* dsp) { delete dsp; }

const static int iNEStingBlockSIG1Wave0[16] = {4068,2034,1016,762,508,380,234,202,160,128,96,64,32,16,8,4};
class NEStingBlockSIG1 {
	
  private:
	
	int iNEStingBlockSIG1Wave0_idx;
	
  public:
	
	int getNumInputsNEStingBlockSIG1() {
		return 0;
	}
	int getNumOutputsNEStingBlockSIG1() {
		return 1;
	}
	int getInputRateNEStingBlockSIG1(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRateNEStingBlockSIG1(int channel) {
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
	
	void instanceInitNEStingBlockSIG1(int sample_rate) {
		iNEStingBlockSIG1Wave0_idx = 0;
	}
	
	void fillNEStingBlockSIG1(int count, int* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			table[i] = iNEStingBlockSIG1Wave0[iNEStingBlockSIG1Wave0_idx];
			iNEStingBlockSIG1Wave0_idx = ((1 + iNEStingBlockSIG1Wave0_idx) % 16);
		}
	}

};

static NEStingBlockSIG1* newNEStingBlockSIG1() { return (NEStingBlockSIG1*)new NEStingBlockSIG1(); }
static void deleteNEStingBlockSIG1(NEStingBlockSIG1* dsp) { delete dsp; }

class NEStingBlockSIG2 {
	
  private:
	
	int iRec3[2];
	
  public:
	
	int getNumInputsNEStingBlockSIG2() {
		return 0;
	}
	int getNumOutputsNEStingBlockSIG2() {
		return 1;
	}
	int getInputRateNEStingBlockSIG2(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRateNEStingBlockSIG2(int channel) {
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
	
	void instanceInitNEStingBlockSIG2(int sample_rate) {
		for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
			iRec3[l7] = 0;
		}
	}
	
	void fillNEStingBlockSIG2(int count, float* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			iRec3[0] = (iRec3[1] + 1);
			table[i] = std::sin((9.58738019e-05f * float((iRec3[0] + -1))));
			iRec3[1] = iRec3[0];
		}
	}

};

static NEStingBlockSIG2* newNEStingBlockSIG2() { return (NEStingBlockSIG2*)new NEStingBlockSIG2(); }
static void deleteNEStingBlockSIG2(NEStingBlockSIG2* dsp) { delete dsp; }

static float NEStingBlock_faustpower2_f(float value) {
	return (value * value);
}
static float ftbl0NEStingBlockSIG0[512];
static int itbl1NEStingBlockSIG1[16];
static float ftbl2NEStingBlockSIG2[65536];

#ifndef FAUSTCLASS 
#define FAUSTCLASS NEStingBlock
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

class NEStingBlock : public dsp {
	
 private:
	
	FAUSTFLOAT fButton0;
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst0;
	float fConst1;
	FAUSTFLOAT fEntry1;
	float fVec0[2];
	int iVec1[2];
	float fConst2;
	float fRec0[2];
	float fVec2[2];
	int IOTA;
	float fVec3[8192];
	FAUSTFLOAT fEntry2;
	float fRec1[2];
	FAUSTFLOAT fEntry3;
	int iRec2[2];
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.1");
		m->declare("filename", "NESted.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.1");
		m->declare("name", "Triangle2A03");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "0.0");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	virtual int getInputRate(int channel) {
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
		NEStingBlockSIG0* sig0 = newNEStingBlockSIG0();
		sig0->instanceInitNEStingBlockSIG0(sample_rate);
		sig0->fillNEStingBlockSIG0(512, ftbl0NEStingBlockSIG0);
		NEStingBlockSIG1* sig1 = newNEStingBlockSIG1();
		sig1->instanceInitNEStingBlockSIG1(sample_rate);
		sig1->fillNEStingBlockSIG1(16, itbl1NEStingBlockSIG1);
		NEStingBlockSIG2* sig2 = newNEStingBlockSIG2();
		sig2->instanceInitNEStingBlockSIG2(sample_rate);
		sig2->fillNEStingBlockSIG2(65536, ftbl2NEStingBlockSIG2);
		deleteNEStingBlockSIG0(sig0);
		deleteNEStingBlockSIG1(sig1);
		deleteNEStingBlockSIG2(sig2);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = (0.25f * fConst0);
		fConst2 = (1.0f / fConst0);
	}
	
	virtual void instanceResetUserInterface() {
		fButton0 = FAUSTFLOAT(0.0f);
		fEntry0 = FAUSTFLOAT(1.0f);
		fEntry1 = FAUSTFLOAT(440.0f);
		fEntry2 = FAUSTFLOAT(2.0f);
		fEntry3 = FAUSTFLOAT(0.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			iVec1[l1] = 0;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec0[l2] = 0.0f;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fVec2[l3] = 0.0f;
		}
		IOTA = 0;
		for (int l4 = 0; (l4 < 8192); l4 = (l4 + 1)) {
			fVec3[l4] = 0.0f;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec1[l5] = 0.0f;
		}
		for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
			iRec2[l6] = 0;
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
	
	virtual NEStingBlock* clone() {
		return new NEStingBlock();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("Triangle2A03");
		ui_interface->addNumEntry("PulseWidth", &fEntry2, 2.0f, 0.0f, 3.0f, 1.0f);
		ui_interface->addNumEntry("TimerMode", &fEntry3, 0.0f, 0.0f, 1.0f, 1.0f);
		ui_interface->addNumEntry("WaveShape", &fEntry0, 1.0f, 0.0f, 3.0f, 1.0f);
		ui_interface->declare(&fEntry1, "unit", "Hz");
		ui_interface->addNumEntry("freq", &fEntry1, 440.0f, 20.0f, 20000.0f, 1.0f);
		ui_interface->addButton("gate", &fButton0);
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = float(fButton0);
		float fSlow1 = float(fEntry0);
		int iSlow2 = (fSlow1 >= 2.0f);
		int iSlow3 = (fSlow1 >= 1.0f);
		float fSlow4 = float(fEntry1);
		float fSlow5 = std::max<float>(20.0f, std::fabs(fSlow4));
		float fSlow6 = (fConst1 / fSlow5);
		float fSlow7 = float(fEntry2);
		int iSlow8 = std::max<int>(0, std::min<int>(8191, int(((fConst0 / fSlow4) / ((fSlow7 >= 2.0f) ? ((fSlow7 >= 3.0f) ? 0.75f : 0.5f) : ((fSlow7 >= 1.0f) ? 0.25f : 0.125f))))));
		float fSlow9 = (fConst2 * fSlow4);
		int iSlow10 = (fSlow1 >= 3.0f);
		int iSlow11 = std::max<int>(1, (int(float(fEntry3)) ? 93 : 32767));
		float fSlow12 = std::fmod(fSlow4, 16.0f);
		float fSlow13 = (2013494.0f / fSlow12);
		float fSlow14 = (2237215.5f / fSlow12);
		float fSlow15 = (2460937.25f / fSlow12);
		for (int i = 0; (i < count); i = (i + 1)) {
			fVec0[0] = fSlow5;
			iVec1[0] = 1;
			float fTemp0 = (fRec0[1] + (fConst2 * fVec0[1]));
			fRec0[0] = (fTemp0 - std::floor(fTemp0));
			float fTemp1 = NEStingBlock_faustpower2_f(((2.0f * fRec0[0]) + -1.0f));
			fVec2[0] = fTemp1;
			float fTemp2 = (fSlow6 * (float(iVec1[1]) * (fTemp1 - fVec2[1])));
			fVec3[(IOTA & 8191)] = fTemp2;
			fRec1[0] = (fSlow9 + (fRec1[1] - std::floor((fSlow9 + fRec1[1]))));
			iRec2[0] = ((iRec2[1] + 1) % iSlow11);
			int iTemp3 = (iRec2[0] % 4);
			float fTemp4 = float(iRec2[0]);
			float fTemp5 = ftbl2NEStingBlockSIG2[int((65536.0f * fRec1[0]))];
			output0[i] = FAUSTFLOAT((fSlow0 * (iSlow2 ? (iSlow10 ? float(input0[i]) : ((iTemp3 >= 2) ? ((iTemp3 >= 3) ? (float(itbl1NEStingBlockSIG1[(int((fTemp4 + (fSlow15 / fTemp5))) % 16)]) + 0.100000001f) : (float(itbl1NEStingBlockSIG1[(int((fTemp4 + (fSlow14 / fTemp5))) % 16)]) + 0.100000001f)) : ((iTemp3 >= 1) ? (float(itbl1NEStingBlockSIG1[(int((fTemp4 + (fSlow13 / fTemp5))) % 16)]) + 0.100000001f) : (float(itbl1NEStingBlockSIG1[(int(fTemp4) % 16)]) + 0.100000001f)))) : (iSlow3 ? ((0.125f * ftbl0NEStingBlockSIG0[int((512.0f * fRec1[0]))]) + -1.0f) : (fTemp2 - fVec3[((IOTA - iSlow8) & 8191)])))));
			fVec0[1] = fVec0[0];
			iVec1[1] = iVec1[0];
			fRec0[1] = fRec0[0];
			fVec2[1] = fVec2[0];
			IOTA = (IOTA + 1);
			fRec1[1] = fRec1[0];
			iRec2[1] = iRec2[0];
		}
	}

};

#endif
