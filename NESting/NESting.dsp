
declare name "SquareTri2A03";
import("stdfaust.lib");
import("soundfiles.lib");

declare options "[midi:on]";

grGeneral(x) = vgroup("Main", hgroup("General", x));
grRow2(x) = vgroup("Main", hgroup("Row 2", x));

midigate = grGeneral(checkbox("h:gate"));
midifreq = grGeneral(nentry("freq[unit:Hz]", 440, 20, 20000, 1));
midigain = grGeneral(hslider("gain", 0.5, 0, 1, 0.01));
waveshape = grRow2(nentry("WaveShape", 0, 0, 3, 1));
pulsewidth = grRow2(nentry("PulseWidth", 2, 0, 4.1, 1)) : si.smoo;

// Unused, but maybe useful later
clock_cycle = 1789773; // 1.789773 MHz
clock_samples = ba.sec2samp(1. / (clock_cycle)); // 2^11 = 2048 CPU cycles per phase value


lerp(low, high, x) = (x - low) / (high - low);
unlerp(low, high, x) = (x * (high - low)) + low;

//////
// Square Wave Generator
//////

squareValue = saw1 + saw2
with {
  phase_table = waveform{0.06, 0.12, 0.25, 0.50, 0.75};
  phase1 = rdtable(phase_table, int(pulsewidth));
  saw1 = os.sawNp(2, midifreq, 0) : fi.lowpass(2, 13000);
  saw2 = os.sawNp(2, midifreq, phase1) * -1 : fi.lowpass(2, 13000);
};

//////
// Triangle Wave Generator
//////

triangleValue = (rdtable(triWave,int(os.phasor(512,midifreq))) * 0.125) - 1.
with {
  // From https://www.mattmontag.com/nesvst/triangle_wavetable.txt
  triWave = waveform{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
	14.58, 14.58, 14.58, 14.58, 14.58, 14.58, 14.58, 14.58, 
	14.58, 14.58, 14.58, 14.58, 14.58, 14.58, 14.58, 14.58, 
	13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 13.1, 
	11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 11.46, 
	9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 9.88, 
	8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 8.7, 
	7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 7.66, 
	6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 6.64, 
	5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 5.70, 
	4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 4.76, 
	3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 3.82, 
	2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 2.88, 
	2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 2.18, 
	1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 
	.76, .76, .76, .76, .76, .76, .76, .76, .76, .76, .76, .76, .76, .76, .76, .76, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
};


square2Value = os.square(midifreq);

// Noise is implemented in C++.

//////
// DPCM Generator
//////

dpcmValue = _ : dpcmFilter
with {
  // We assume that our sample is already at the correct sample rate.
  // If not, the host will have to re-sample it.
  //isample = soundfile( "[url:http://example.com/test.mp3]", 1 );
  dpcmFilter = _ : fi.low_shelf(dGain, lCutoff) : fi.high_shelf(dGain, hCutoff);
  lCutoff = 0.9;
  lSlope = 0.7;
  dGain = 0.1;
  hCutoff = 0.2;
  hSlope = 0.7;
};

process = (ba.selectn(4, waveshape, (squareValue, triangleValue, square2Value, dpcmValue)) * midigain * midigate) <: (_,_) ;

