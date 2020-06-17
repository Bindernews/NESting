/* ------------------------------------------------------------
name: "SquareTri2A03"
Code generated with Faust 2.20.2 (https://faust.grame.fr)
Compilation options: -lang cpp -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __Faust1_H__
#define  __Faust1_H__


#define FAUSTFLOAT iplug::sample

#include "IPlugFaust.h"
/************************** BEGIN audio.h **************************/
/************************************************************************
 FAUST Architecture File
 Copyright (C) 2003-2017 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This Architecture section is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3 of
 the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; If not, see <http://www.gnu.org/licenses/>.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __audio__
#define __audio__

#include <set>
#include <utility>

class dsp;

typedef void (* shutdown_callback)(const char* message, void* arg);

typedef void (* compute_callback)(void* arg);

class audio {
    
    protected:
    
        shutdown_callback fShutdown;    // Shutdown callback
        void* fShutdownArg;             // Shutdown callback data
    
        std::set<std::pair<compute_callback, void*> > fComputeCallbackList;
    
    public:
    
        audio():fShutdown(nullptr), fShutdownArg(nullptr) {}
        virtual ~audio() {}

        virtual bool init(const char* name, dsp* dsp) = 0;
    
        virtual bool start() = 0;
        virtual void stop() = 0;
    
        void setShutdownCallback(shutdown_callback cb, void* arg)
        {
            fShutdown = cb;
            fShutdownArg = arg;
        }
    
        void addControlCallback(compute_callback cb, void* arg)
        {
            fComputeCallbackList.insert(std::make_pair(cb, arg));
        }
        bool removeControlCallback(compute_callback cb, void* arg)
        {
            return (fComputeCallbackList.erase(std::make_pair(cb, arg)) == 1);
        }
        void runControlCallbacks()
        {
            for (auto& it : fComputeCallbackList) {
                it.first(it.second);
            }
        }
    
        virtual int getBufferSize() = 0;
        virtual int getSampleRate() = 0;

        virtual int getNumInputs() = 0;
        virtual int getNumOutputs() = 0;

        virtual float getCPULoad() { return 0.f; }
};
					
#endif
/**************************  END  audio.h **************************/
/************************** BEGIN dsp.h **************************/
/************************************************************************
 FAUST Architecture File
 Copyright (C) 2003-2017 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This Architecture section is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3 of
 the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; If not, see <http://www.gnu.org/licenses/>.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __dsp__
#define __dsp__

#include <string>
#include <vector>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

class UI;
struct Meta;

/**
 * DSP memory manager.
 */

struct dsp_memory_manager {
    
    virtual ~dsp_memory_manager() {}
    
    virtual void* allocate(size_t size) = 0;
    virtual void destroy(void* ptr) = 0;
    
};

/**
* Signal processor definition.
*/

class dsp {

    public:

        dsp() {}
        virtual ~dsp() {}

        /* Return instance number of audio inputs */
        virtual int getNumInputs() = 0;
    
        /* Return instance number of audio outputs */
        virtual int getNumOutputs() = 0;
    
        /**
         * Trigger the ui_interface parameter with instance specific calls
         * to 'addBtton', 'addVerticalSlider'... in order to build the UI.
         *
         * @param ui_interface - the user interface builder
         */
        virtual void buildUserInterface(UI* ui_interface) = 0;
    
        /* Returns the sample rate currently used by the instance */
        virtual int getSampleRate() = 0;
    
        /**
         * Global init, calls the following methods:
         * - static class 'classInit': static tables initialization
         * - 'instanceInit': constants and instance state initialization
         *
         * @param sample_rate - the sampling rate in Hertz
         */
        virtual void init(int sample_rate) = 0;

        /**
         * Init instance state
         *
         * @param sample_rate - the sampling rate in Hertz
         */
        virtual void instanceInit(int sample_rate) = 0;

        /**
         * Init instance constant state
         *
         * @param sample_rate - the sampling rate in Hertz
         */
        virtual void instanceConstants(int sample_rate) = 0;
    
        /* Init default control parameters values */
        virtual void instanceResetUserInterface() = 0;
    
        /* Init instance state (delay lines...) */
        virtual void instanceClear() = 0;
 
        /**
         * Return a clone of the instance.
         *
         * @return a copy of the instance on success, otherwise a null pointer.
         */
        virtual dsp* clone() = 0;
    
        /**
         * Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
         *
         * @param m - the Meta* meta user
         */
        virtual void metadata(Meta* m) = 0;
    
        /**
         * DSP instance computation, to be called with successive in/out audio buffers.
         *
         * @param count - the number of frames to compute
         * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)
         * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)
         *
         */
        virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;
    
        /**
         * DSP instance computation: alternative method to be used by subclasses.
         *
         * @param date_usec - the timestamp in microsec given by audio driver.
         * @param count - the number of frames to compute
         * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad)
         * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad)
         *
         */
        virtual void compute(double /*date_usec*/, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) { compute(count, inputs, outputs); }
       
};

/**
 * Generic DSP decorator.
 */

class decorator_dsp : public dsp {

    protected:

        dsp* fDSP;

    public:

        decorator_dsp(dsp* dsp = nullptr):fDSP(dsp) {}
        virtual ~decorator_dsp() { delete fDSP; }

        virtual int getNumInputs() { return fDSP->getNumInputs(); }
        virtual int getNumOutputs() { return fDSP->getNumOutputs(); }
        virtual void buildUserInterface(UI* ui_interface) { fDSP->buildUserInterface(ui_interface); }
        virtual int getSampleRate() { return fDSP->getSampleRate(); }
        virtual void init(int sample_rate) { fDSP->init(sample_rate); }
        virtual void instanceInit(int sample_rate) { fDSP->instanceInit(sample_rate); }
        virtual void instanceConstants(int sample_rate) { fDSP->instanceConstants(sample_rate); }
        virtual void instanceResetUserInterface() { fDSP->instanceResetUserInterface(); }
        virtual void instanceClear() { fDSP->instanceClear(); }
        virtual decorator_dsp* clone() { return new decorator_dsp(fDSP->clone()); }
        virtual void metadata(Meta* m) { fDSP->metadata(m); }
        // Beware: subclasses usually have to overload the two 'compute' methods
        virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) { fDSP->compute(count, inputs, outputs); }
        virtual void compute(double date_usec, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) { fDSP->compute(date_usec, count, inputs, outputs); }
    
};

/**
 * DSP factory class.
 */

class dsp_factory {
    
    protected:
    
        // So that to force sub-classes to use deleteDSPFactory(dsp_factory* factory);
        virtual ~dsp_factory() {}
    
    public:
    
        virtual std::string getName() = 0;
        virtual std::string getSHAKey() = 0;
        virtual std::string getDSPCode() = 0;
        virtual std::string getCompileOptions() = 0;
        virtual std::vector<std::string> getLibraryList() = 0;
        virtual std::vector<std::string> getIncludePathnames() = 0;
    
        virtual dsp* createDSPInstance() = 0;
    
        virtual void setMemoryManager(dsp_memory_manager* manager) = 0;
        virtual dsp_memory_manager* getMemoryManager() = 0;
    
};

/**
 * On Intel set FZ (Flush to Zero) and DAZ (Denormals Are Zero)
 * flags to avoid costly denormals.
 */

#ifdef __SSE__
    #include <xmmintrin.h>
    #ifdef __SSE2__
        #define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8040)
    #else
        #define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8000)
    #endif
#else
    #define AVOIDDENORMALS
#endif

#endif
/**************************  END  dsp.h **************************/
/************************** BEGIN dsp-adapter.h **************************/
/************************************************************************
 FAUST Architecture File
 Copyright (C) 2003-2017 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This Architecture section is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3 of
 the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; If not, see <http://www.gnu.org/licenses/>.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __dsp_adapter__
#define __dsp_adapter__

#include <string.h>
#include <iostream>


// Adapts a DSP for a different number of inputs/outputs

class dsp_adapter : public decorator_dsp {
    
    private:
        
        FAUSTFLOAT** fAdaptedInputs;
        FAUSTFLOAT** fAdaptedOutputs;
        int fHardwareInputs;
        int fHardwareOutputs;
        
        void adaptBuffers(FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
        {
            for (int i = 0; i < fHardwareInputs; i++) {
                fAdaptedInputs[i] = inputs[i];
            }
            for (int i = 0; i < fHardwareOutputs; i++) {
                fAdaptedOutputs[i] = outputs[i];
            }
        }
        
    public:
        
        dsp_adapter(dsp* dsp, int hardware_inputs, int hardware_outputs, int buffer_size):decorator_dsp(dsp)
        {
            fHardwareInputs = hardware_inputs;
            fHardwareOutputs = hardware_outputs;
             
            fAdaptedInputs = new FAUSTFLOAT*[dsp->getNumInputs()];
            for (int i = 0; i < dsp->getNumInputs() - fHardwareInputs; i++) {
                fAdaptedInputs[i + fHardwareInputs] = new FAUSTFLOAT[buffer_size];
                memset(fAdaptedInputs[i + fHardwareInputs], 0, sizeof(FAUSTFLOAT) * buffer_size);
            }
            
            fAdaptedOutputs = new FAUSTFLOAT*[dsp->getNumOutputs()];
            for (int i = 0; i < dsp->getNumOutputs() - fHardwareOutputs; i++) {
                fAdaptedOutputs[i + fHardwareOutputs] = new FAUSTFLOAT[buffer_size];
                memset(fAdaptedOutputs[i + fHardwareOutputs], 0, sizeof(FAUSTFLOAT) * buffer_size);
            }
        }
        
        virtual ~dsp_adapter()
        {
            for (int i = 0; i < fDSP->getNumInputs() - fHardwareInputs; i++) {
                delete [] fAdaptedInputs[i + fHardwareInputs];
            }
            delete [] fAdaptedInputs;
            
            for (int i = 0; i < fDSP->getNumOutputs() - fHardwareOutputs; i++) {
                delete [] fAdaptedOutputs[i + fHardwareOutputs];
            }
            delete [] fAdaptedOutputs;
        }
    
        virtual int getNumInputs() { return fHardwareInputs; }
        virtual int getNumOutputs() { return fHardwareOutputs; }
    
        virtual void compute(double date_usec, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
        {
            adaptBuffers(inputs, outputs);
            fDSP->compute(date_usec, count, fAdaptedInputs, fAdaptedOutputs);
        }
        
        virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
        {
            adaptBuffers(inputs, outputs);
            fDSP->compute(count, fAdaptedInputs, fAdaptedOutputs);
        }
};

// Adapts a DSP for a different sample size

template <typename TYPE_INT, typename TYPE_EXT>
class dsp_sample_adapter : public decorator_dsp {
    
    protected:
    
        TYPE_INT** fAdaptedInputs;
        TYPE_INT** fAdaptedOutputs;
    
        void adaptInputBuffers(int count, FAUSTFLOAT** inputs)
        {
            for (int chan = 0; chan < fDSP->getNumInputs(); chan++) {
                for (int frame = 0; frame < count; frame++) {
                    fAdaptedInputs[chan][frame] = TYPE_INT(reinterpret_cast<TYPE_EXT**>(inputs)[chan][frame]);
                }
            }
        }
    
        void adaptOutputsBuffers(int count, FAUSTFLOAT** outputs)
        {
            for (int chan = 0; chan < fDSP->getNumOutputs(); chan++) {
                for (int frame = 0; frame < count; frame++) {
                    reinterpret_cast<TYPE_EXT**>(outputs)[chan][frame] = TYPE_EXT(fAdaptedOutputs[chan][frame]);
                }
            }
        }
    
    public:
    
        dsp_sample_adapter(dsp* dsp):decorator_dsp(dsp)
        {
            fAdaptedInputs = new TYPE_INT*[dsp->getNumInputs()];
            for (int i = 0; i < dsp->getNumInputs(); i++) {
                fAdaptedInputs[i] = new TYPE_INT[4096];
            }
            
            fAdaptedOutputs = new TYPE_INT*[dsp->getNumOutputs()];
            for (int i = 0; i < dsp->getNumOutputs(); i++) {
                fAdaptedOutputs[i] = new TYPE_INT[4096];
            }
        }
    
        virtual ~dsp_sample_adapter()
        {
            for (int i = 0; i < fDSP->getNumInputs(); i++) {
                delete [] fAdaptedInputs[i];
            }
            delete [] fAdaptedInputs;
            
            for (int i = 0; i < fDSP->getNumOutputs(); i++) {
                delete [] fAdaptedOutputs[i];
            }
            delete [] fAdaptedOutputs;
        }
    
        virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
        {
            adaptInputBuffers(count, inputs);
            // DSP base class uses FAUSTFLOAT** type, so reinterpret_cast has to be used even if the real DSP uses TYPE_INT
            fDSP->compute(count, reinterpret_cast<FAUSTFLOAT**>(fAdaptedInputs), reinterpret_cast<FAUSTFLOAT**>(fAdaptedOutputs));
            adaptOutputsBuffers(count, outputs);
        }
    
        virtual void compute(double date_usec, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
        {
            adaptInputBuffers(count, inputs);
            // DSP base class uses FAUSTFLOAT** type, so reinterpret_cast has to be used even if the real DSP uses TYPE_INT
            fDSP->compute(date_usec, count, reinterpret_cast<FAUSTFLOAT**>(fAdaptedInputs), reinterpret_cast<FAUSTFLOAT**>(fAdaptedOutputs));
            adaptOutputsBuffers(count, outputs);
       }
};

#endif
/**************************  END  dsp-adapter.h **************************/

/* BEGIN AUTO GENERATED BY THE FAUST COMPILER ... */
//#define max std::max
//#define min std::min
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>

const static float fFaust1SIG0Wave0[5] = {0.0599999987f,0.119999997f,0.25f,0.5f,0.75f};
class Faust1SIG0 {
	
  private:
	
	int fFaust1SIG0Wave0_idx;
	
  public:
	
	int getNumInputsFaust1SIG0() {
		return 0;
	}
	int getNumOutputsFaust1SIG0() {
		return 1;
	}
	int getInputRateFaust1SIG0(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRateFaust1SIG0(int channel) {
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
	
	void instanceInitFaust1SIG0(int sample_rate) {
		fFaust1SIG0Wave0_idx = 0;
	}
	
	void fillFaust1SIG0(int count, float* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			table[i] = fFaust1SIG0Wave0[fFaust1SIG0Wave0_idx];
			fFaust1SIG0Wave0_idx = ((1 + fFaust1SIG0Wave0_idx) % 5);
		}
	}

};

static Faust1SIG0* newFaust1SIG0() { return (Faust1SIG0*)new Faust1SIG0(); }
static void deleteFaust1SIG0(Faust1SIG0* dsp) { delete dsp; }

const static float fFaust1SIG1Wave0[512] = {8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,8.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,9.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,11.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,12.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,13.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,14.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,15.0f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,14.5799999f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,13.1000004f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,11.46f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,9.88000011f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,8.69999981f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,7.65999985f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,6.63999987f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,5.69999981f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,4.76000023f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,3.81999993f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.88000011f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,2.18000007f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,1.46000004f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.75999999f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,2.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,3.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,4.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,5.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,6.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f,7.0f};
class Faust1SIG1 {
	
  private:
	
	int fFaust1SIG1Wave0_idx;
	
  public:
	
	int getNumInputsFaust1SIG1() {
		return 0;
	}
	int getNumOutputsFaust1SIG1() {
		return 1;
	}
	int getInputRateFaust1SIG1(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRateFaust1SIG1(int channel) {
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
	
	void instanceInitFaust1SIG1(int sample_rate) {
		fFaust1SIG1Wave0_idx = 0;
	}
	
	void fillFaust1SIG1(int count, float* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			table[i] = fFaust1SIG1Wave0[fFaust1SIG1Wave0_idx];
			fFaust1SIG1Wave0_idx = ((1 + fFaust1SIG1Wave0_idx) % 512);
		}
	}

};

static Faust1SIG1* newFaust1SIG1() { return (Faust1SIG1*)new Faust1SIG1(); }
static void deleteFaust1SIG1(Faust1SIG1* dsp) { delete dsp; }

static float Faust1_faustpower2_f(float value) {
	return (value * value);
}
static float ftbl0Faust1SIG0[5];
static float ftbl1Faust1SIG1[512];

#ifndef FAUSTCLASS 
#define FAUSTCLASS Faust1
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

class Faust1 : public dsp {
	
 private:
	
	FAUSTFLOAT fCheckbox0;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	float fConst4;
	FAUSTFLOAT fEntry1;
	float fVec0[2];
	int iVec1[2];
	float fConst5;
	float fRec1[2];
	float fVec2[2];
	int IOTA;
	float fVec3[8192];
	float fConst6;
	float fConst7;
	float fRec0[3];
	FAUSTFLOAT fEntry2;
	float fRec3[2];
	float fRec2[3];
	float fRec4[2];
	float fVec4[2];
	float fRec5[2];
	float fVec5[2];
	float fVec6[4096];
	float fConst8;
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	float fConst15;
	float fConst16;
	float fConst17;
	float fConst18;
	float fConst19;
	float fConst20;
	float fVec7[2];
	float fConst21;
	float fConst22;
	float fRec9[2];
	float fConst23;
	float fConst24;
	float fConst25;
	float fRec8[3];
	float fRec11[2];
	float fRec10[3];
	float fVec8[2];
	float fConst26;
	float fRec7[2];
	float fConst27;
	float fConst28;
	float fConst29;
	float fConst30;
	float fRec6[3];
	float fConst31;
	float fConst32;
	float fConst33;
	float fRec13[2];
	float fRec12[3];
	float fConst34;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "0.0");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.1");
		m->declare("filename", "NESting.dsp");
		m->declare("filters.lib/filterbank:author", "Julius O. Smith III");
		m->declare("filters.lib/filterbank:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/filterbank:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/highpass:author", "Julius O. Smith III");
		m->declare("filters.lib/highpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/highshelf:author", "Julius O. Smith III");
		m->declare("filters.lib/highshelf:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/highshelf:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/low_shelf:author", "Julius O. Smith III");
		m->declare("filters.lib/low_shelf:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/low_shelf:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowshelf:author", "Julius O. Smith III");
		m->declare("filters.lib/lowshelf:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowshelf:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/tf1:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2s:license", "MIT-style STK-4.3 license");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.1");
		m->declare("name", "SquareTri2A03");
		m->declare("options", "[midi:on]");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "0.0");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "0.0");
		m->declare("soundfiles.lib/name", "Faust Soundfile Library");
		m->declare("soundfiles.lib/version", "0.6");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 2;
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
			case 1: {
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
		Faust1SIG0* sig0 = newFaust1SIG0();
		sig0->instanceInitFaust1SIG0(sample_rate);
		sig0->fillFaust1SIG0(5, ftbl0Faust1SIG0);
		Faust1SIG1* sig1 = newFaust1SIG1();
		sig1->instanceInitFaust1SIG1(sample_rate);
		sig1->fillFaust1SIG1(512, ftbl1Faust1SIG1);
		deleteFaust1SIG0(sig0);
		deleteFaust1SIG1(sig1);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = std::tan((40840.7031f / fConst0));
		fConst2 = (1.0f / fConst1);
		fConst3 = (1.0f / (((fConst2 + 1.41421354f) / fConst1) + 1.0f));
		fConst4 = (0.25f * fConst0);
		fConst5 = (1.0f / fConst0);
		fConst6 = (((fConst2 + -1.41421354f) / fConst1) + 1.0f);
		fConst7 = (2.0f * (1.0f - (1.0f / Faust1_faustpower2_f(fConst1))));
		fConst8 = (0.5f * fConst0);
		fConst9 = std::tan((0.628318548f / fConst0));
		fConst10 = (1.0f / fConst9);
		fConst11 = (1.0f / (((fConst10 + 1.0f) / fConst9) + 1.0f));
		fConst12 = (fConst10 + 1.0f);
		fConst13 = (1.0f / fConst12);
		fConst14 = std::tan((2.82743335f / fConst0));
		fConst15 = (1.0f / fConst14);
		fConst16 = (1.0f / (((fConst15 + 1.0f) / fConst14) + 1.0f));
		fConst17 = Faust1_faustpower2_f(fConst14);
		fConst18 = (0.0f - (2.0f / fConst17));
		fConst19 = (fConst15 + 1.0f);
		fConst20 = (0.0f - (1.0f / (fConst14 * fConst19)));
		fConst21 = (1.0f / fConst19);
		fConst22 = (1.0f - fConst15);
		fConst23 = (((fConst15 + -1.0f) / fConst14) + 1.0f);
		fConst24 = (1.0f / fConst17);
		fConst25 = (2.0f * (1.0f - fConst24));
		fConst26 = (1.0f - fConst10);
		fConst27 = (((fConst10 + -1.0f) / fConst9) + 1.0f);
		fConst28 = Faust1_faustpower2_f(fConst9);
		fConst29 = (1.0f / fConst28);
		fConst30 = (2.0f * (1.0f - fConst29));
		fConst31 = (1.0f / (fConst9 * fConst12));
		fConst32 = (0.0f - fConst31);
		fConst33 = (fConst26 / fConst12);
		fConst34 = (0.0f - (2.0f / fConst28));
	}
	
	virtual void instanceResetUserInterface() {
		fCheckbox0 = FAUSTFLOAT(0.0f);
		fHslider0 = FAUSTFLOAT(0.5f);
		fEntry0 = FAUSTFLOAT(0.0f);
		fEntry1 = FAUSTFLOAT(440.0f);
		fEntry2 = FAUSTFLOAT(2.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			iVec1[l1] = 0;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec1[l2] = 0.0f;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fVec2[l3] = 0.0f;
		}
		IOTA = 0;
		for (int l4 = 0; (l4 < 8192); l4 = (l4 + 1)) {
			fVec3[l4] = 0.0f;
		}
		for (int l5 = 0; (l5 < 3); l5 = (l5 + 1)) {
			fRec0[l5] = 0.0f;
		}
		for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
			fRec3[l6] = 0.0f;
		}
		for (int l7 = 0; (l7 < 3); l7 = (l7 + 1)) {
			fRec2[l7] = 0.0f;
		}
		for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
			fRec4[l8] = 0.0f;
		}
		for (int l9 = 0; (l9 < 2); l9 = (l9 + 1)) {
			fVec4[l9] = 0.0f;
		}
		for (int l10 = 0; (l10 < 2); l10 = (l10 + 1)) {
			fRec5[l10] = 0.0f;
		}
		for (int l11 = 0; (l11 < 2); l11 = (l11 + 1)) {
			fVec5[l11] = 0.0f;
		}
		for (int l12 = 0; (l12 < 4096); l12 = (l12 + 1)) {
			fVec6[l12] = 0.0f;
		}
		for (int l13 = 0; (l13 < 2); l13 = (l13 + 1)) {
			fVec7[l13] = 0.0f;
		}
		for (int l14 = 0; (l14 < 2); l14 = (l14 + 1)) {
			fRec9[l14] = 0.0f;
		}
		for (int l15 = 0; (l15 < 3); l15 = (l15 + 1)) {
			fRec8[l15] = 0.0f;
		}
		for (int l16 = 0; (l16 < 2); l16 = (l16 + 1)) {
			fRec11[l16] = 0.0f;
		}
		for (int l17 = 0; (l17 < 3); l17 = (l17 + 1)) {
			fRec10[l17] = 0.0f;
		}
		for (int l18 = 0; (l18 < 2); l18 = (l18 + 1)) {
			fVec8[l18] = 0.0f;
		}
		for (int l19 = 0; (l19 < 2); l19 = (l19 + 1)) {
			fRec7[l19] = 0.0f;
		}
		for (int l20 = 0; (l20 < 3); l20 = (l20 + 1)) {
			fRec6[l20] = 0.0f;
		}
		for (int l21 = 0; (l21 < 2); l21 = (l21 + 1)) {
			fRec13[l21] = 0.0f;
		}
		for (int l22 = 0; (l22 < 3); l22 = (l22 + 1)) {
			fRec12[l22] = 0.0f;
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
	
	virtual Faust1* clone() {
		return new Faust1();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("Main");
		ui_interface->openHorizontalBox("General");
		ui_interface->declare(&fEntry1, "unit", "Hz");
		ui_interface->addNumEntry("freq", &fEntry1, 440.0f, 20.0f, 20000.0f, 1.0f);
		ui_interface->addHorizontalSlider("gain", &fHslider0, 0.5f, 0.0f, 1.0f, 0.00999999978f);
		ui_interface->openHorizontalBox("gate");
		ui_interface->addCheckButton("0x00", &fCheckbox0);
		ui_interface->closeBox();
		ui_interface->closeBox();
		ui_interface->openHorizontalBox("Row 2");
		ui_interface->addNumEntry("PulseWidth", &fEntry2, 2.0f, 0.0f, 4.0999999f, 1.0f);
		ui_interface->addNumEntry("WaveShape", &fEntry0, 0.0f, 0.0f, 3.0f, 1.0f);
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = (float(fCheckbox0) * float(fHslider0));
		float fSlow1 = float(fEntry0);
		int iSlow2 = (fSlow1 >= 2.0f);
		int iSlow3 = (fSlow1 >= 1.0f);
		float fSlow4 = float(fEntry1);
		float fSlow5 = std::max<float>(20.0f, std::fabs(fSlow4));
		float fSlow6 = (fConst4 / fSlow5);
		float fSlow7 = (fConst0 / fSlow4);
		float fSlow8 = (0.00100000005f * float(fEntry2));
		float fSlow9 = (fConst5 * fSlow4);
		int iSlow10 = (fSlow1 >= 3.0f);
		float fSlow11 = std::max<float>(fSlow4, 23.4489498f);
		float fSlow12 = std::max<float>(20.0f, std::fabs(fSlow11));
		float fSlow13 = (fConst4 / fSlow12);
		float fSlow14 = std::max<float>(0.0f, std::min<float>(2047.0f, (fConst8 / fSlow11)));
		float fSlow15 = std::floor(fSlow14);
		float fSlow16 = (fSlow15 + (1.0f - fSlow14));
		int iSlow17 = int(fSlow14);
		float fSlow18 = (fSlow14 - fSlow15);
		int iSlow19 = (iSlow17 + 1);
		for (int i = 0; (i < count); i = (i + 1)) {
			fVec0[0] = fSlow5;
			iVec1[0] = 1;
			float fTemp0 = float(iVec1[1]);
			float fTemp1 = (fRec1[1] + (fConst5 * fVec0[1]));
			fRec1[0] = (fTemp1 - std::floor(fTemp1));
			float fTemp2 = Faust1_faustpower2_f(((2.0f * fRec1[0]) + -1.0f));
			fVec2[0] = fTemp2;
			float fTemp3 = (fSlow6 * (fTemp0 * (fTemp2 - fVec2[1])));
			fVec3[(IOTA & 8191)] = fTemp3;
			fRec0[0] = (fTemp3 - (fConst3 * ((fConst6 * fRec0[2]) + (fConst7 * fRec0[1]))));
			fRec3[0] = (fSlow8 + (0.999000013f * fRec3[1]));
			fRec2[0] = (0.0f - (fVec3[((IOTA - std::max<int>(0, std::min<int>(8191, int((fSlow7 * ftbl0Faust1SIG0[int(fRec3[0])]))))) & 8191)] + (fConst3 * ((fConst6 * fRec2[2]) + (fConst7 * fRec2[1])))));
			fRec4[0] = (fSlow9 + (fRec4[1] - std::floor((fSlow9 + fRec4[1]))));
			fVec4[0] = fSlow12;
			float fTemp4 = ((fConst5 * fVec4[1]) + fRec5[1]);
			fRec5[0] = (fTemp4 - std::floor(fTemp4));
			float fTemp5 = Faust1_faustpower2_f(((2.0f * fRec5[0]) + -1.0f));
			fVec5[0] = fTemp5;
			float fTemp6 = (fSlow13 * (fTemp0 * (fTemp5 - fVec5[1])));
			fVec6[(IOTA & 4095)] = fTemp6;
			float fTemp7 = float(input0[i]);
			fVec7[0] = fTemp7;
			fRec9[0] = ((fConst20 * fVec7[1]) + (fConst21 * ((fConst15 * fTemp7) - (fConst22 * fRec9[1]))));
			fRec8[0] = (fRec9[0] - (fConst16 * ((fConst23 * fRec8[2]) + (fConst25 * fRec8[1]))));
			fRec11[0] = (0.0f - (fConst21 * ((fConst22 * fRec11[1]) - (fTemp7 + fVec7[1]))));
			fRec10[0] = (fRec11[0] - (fConst16 * ((fConst23 * fRec10[2]) + (fConst25 * fRec10[1]))));
			float fTemp8 = ((((fConst18 * fRec8[1]) + (fConst24 * fRec8[0])) + (fConst24 * fRec8[2])) + (1.01157951f * (fRec10[2] + (fRec10[0] + (2.0f * fRec10[1])))));
			fVec8[0] = fTemp8;
			fRec7[0] = (fConst13 * ((fConst16 * (fTemp8 + fVec8[1])) - (fConst26 * fRec7[1])));
			fRec6[0] = (fRec7[0] - (fConst11 * ((fConst27 * fRec6[2]) + (fConst30 * fRec6[1]))));
			fRec13[0] = ((fConst16 * ((fConst31 * fTemp8) + (fConst32 * fVec8[1]))) - (fConst33 * fRec13[1]));
			fRec12[0] = (fRec13[0] - (fConst11 * ((fConst27 * fRec12[2]) + (fConst30 * fRec12[1]))));
			float fTemp9 = (fSlow0 * (iSlow2 ? (iSlow10 ? (fConst11 * ((fRec6[2] + (fRec6[0] + (2.0f * fRec6[1]))) + (1.01157951f * (((fConst29 * fRec12[0]) + (fConst34 * fRec12[1])) + (fConst29 * fRec12[2]))))) : (fTemp6 - ((fSlow16 * fVec6[((IOTA - iSlow17) & 4095)]) + (fSlow18 * fVec6[((IOTA - iSlow19) & 4095)])))) : (iSlow3 ? ((0.125f * ftbl1Faust1SIG1[int((512.0f * fRec4[0]))]) + -1.0f) : (fConst3 * ((fRec0[2] + (fRec0[0] + (2.0f * fRec0[1]))) + (fRec2[2] + (fRec2[0] + (2.0f * fRec2[1]))))))));
			output0[i] = FAUSTFLOAT(fTemp9);
			output1[i] = FAUSTFLOAT(fTemp9);
			fVec0[1] = fVec0[0];
			iVec1[1] = iVec1[0];
			fRec1[1] = fRec1[0];
			fVec2[1] = fVec2[0];
			IOTA = (IOTA + 1);
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			fRec3[1] = fRec3[0];
			fRec2[2] = fRec2[1];
			fRec2[1] = fRec2[0];
			fRec4[1] = fRec4[0];
			fVec4[1] = fVec4[0];
			fRec5[1] = fRec5[0];
			fVec5[1] = fVec5[0];
			fVec7[1] = fVec7[0];
			fRec9[1] = fRec9[0];
			fRec8[2] = fRec8[1];
			fRec8[1] = fRec8[0];
			fRec11[1] = fRec11[0];
			fRec10[2] = fRec10[1];
			fRec10[1] = fRec10[0];
			fVec8[1] = fVec8[0];
			fRec7[1] = fRec7[0];
			fRec6[2] = fRec6[1];
			fRec6[1] = fRec6[0];
			fRec13[1] = fRec13[0];
			fRec12[2] = fRec12[1];
			fRec12[1] = fRec12[0];
		}
	}

};
//#undef max
//#undef min
/* ... END AUTO GENERATED BY THE FAUST COMPILER  */
using namespace iplug;

class Faust_Faust1 : public IPlugFaust
{
public:
	Faust_Faust1(const char* name, const char* inputDSPFile = 0, int nVoices = 1, int rate = 1,
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
