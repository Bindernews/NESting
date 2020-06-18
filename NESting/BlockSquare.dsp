import("stdfaust.lib");

grGeneral(x) = hgroup("1", x);

midifreq = grGeneral(hslider("freq[unit:Hz]", 440, 20, 20000, 1));
midigain = grGeneral(hslider("gain", 0.5, 0, 1, 0.01));
pulsewidth = grGeneral(nentry("PulseWidth", 2, 0, 5, 1));

squareValue = saw1 + saw2
with {
  phase_table = waveform{0.06, 0.12, 0.25, 0.50, 0.75};
  phase1 = rdtable(phase_table, int(pulsewidth));
  saw1 = os.sawNp(2, midifreq, 0) : fi.lowpass(2, 13000);
  saw2 = os.sawNp(2, midifreq, phase1) * -1 : fi.lowpass(2, 13000);
};

process = squareValue * midigain;
