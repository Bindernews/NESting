declare name "Triangle2A03";
import("stdfaust.lib");


midigate = checkbox("gate");
midifreq = nentry("freq[unit:Hz]", 440, 20, 20000, 1);
midigain = nentry("gain", 0.5, 0, 1, 0.01);
miditempo = nentry("tempo", 120, 60, 200, 10);
waveshape = nentry("WaveShape", 0, 0, 3, 1);
pulsewidth = nentry("PulseWidth", 2, 0, 4, 1);
timermode = nentry("TimerMode", 0, 0, 1, 1);

// Unused, but maybe useful later
clock_cycle = 1789773; // 1.789773 MHz
clock_samples = ba.sec2samp(1. / (clock_cycle)); // 2^11 = 2048 CPU cycles per phase value

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


//////
// Noise Generator
//////

outInt(ctr, phase) = (ctr + (phase * timestep) + 0.1)
with {
  someTable = waveform{4068, 2034, 1016, 762, 508, 380, 234, 202, 160, 128, 96, 64, 32, 16, 8, 4};
  tphase = os.oscsin(midifreq);
  timestep = 3579545 / ((tphase * 2) * rdtable(someTable, int(midifreq % 16)));
};


noiseValue = ba.selectn(8, ctr % 8, (out1, out2, out3, out4, out5, out6, out7, out8) )
with {
  loopSize = select2(timermode, (32767, 93));
  ctr = ba.sweep(loopSize, 1);
  out1 = outInt(ctr, 0    );
  out2 = outInt(ctr, 1.125);
  out3 = outInt(ctr, 1.25 );
  out4 = outInt(ctr, 1.375);
  out5 = outInt(ctr, 1.5  );
  out6 = outInt(ctr, 1.625);
  out7 = outInt(ctr, 1.75 );
  out8 = outInt(ctr, 1.875);
  // Pick a different sample value each time to help reduce aliasing, I think???
};

//////
// DPCM Generator
//////

dpcmValue = _;

process = ba.selectn(4, waveshape, (squareValue, triangleValue, noiseValue, dpcmValue)) * midigate;

