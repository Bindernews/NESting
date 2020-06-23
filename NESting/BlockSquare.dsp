import("stdfaust.lib");

generate(gain, freq, duty_cycle) = (saw1 + saw2) * gain
with {
  phase_table = waveform{0.06, 0.12, 0.25, 0.50, 0.75};
  phase1 = rdtable(phase_table, int(duty_cycle * 4.99));
  saw1 = os.sawNp(2, freq, 0) : fi.lowpass(2, 13000);
  saw2 = os.sawNp(2, freq, phase1) * -1 : fi.lowpass(2, 13000);
};

// Inputs: { gain (norm), freq (Hz), duty cycle (norm) }
// Outputs: { sound (mono) }
process = (_,_,_) : generate : _;
