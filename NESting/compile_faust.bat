@REM Compile the DSP into a CPP file for iPlug2
"C:\Program Files\Faust\bin\faust" -i -a ./faust/IPlugFaust_arch.cpp -cn Faust1 -lang cpp -o FaustCode.hpp NESting.dsp  
pause