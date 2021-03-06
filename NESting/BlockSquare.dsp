import("stdfaust.lib");
//declare options "[nvoices:4]";

generate(gain, freq, duty_cycle) = (saw1 + saw2) * gain
with {
  cutoff_freq = 12000;
  phase_table = waveform{0.06, 0.12, 0.25, 0.50, 0.75};
  phase1 = rdtable(phase_table, int(duty_cycle * 4.99));
  saw1 = os.sawNp(2, freq, 0) : fi.lowpass(2, cutoff_freq);
  saw2 = os.sawNp(2, freq, phase1) * -1 : fi.lowpass(2, cutoff_freq);
};

// Inputs: { gain (norm), freq (Hz), duty cycle (norm) }
// Outputs: { sound (mono) }
process = (_,_,_) : generate : _;
