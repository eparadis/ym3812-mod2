### YM3812-mod2

This is the firmware for an arduino pro mini clone connected to an YM3812 (OPL2) FM synth chip.

There is a 1U eurorack tile format front panel [here](https://github.com/eparadis/eurorack_scad/blob/main/one_u_tiles/opl2_vco.scad) in OpenSCAD format.

A PCB set is forth-coming. The main board has already been designed and tested. The secondary board with analog signal path, power conditioning, etc is not yet designed.

### CV Inputs
- pitch: currently quantized due to limitation of the ADC on the arduino pro mini (or whatever this close is supposed to be)
- modulation: amplitude of OP1 into OP2
- multiplier ratio: All possible ratios of frequency multipliers are calculated and sorted and presented in order on a single CV input

### TODO
- explore various modes of polyphony using the other 8 (!) voices on the chip
- use I2C to connect a human interface for tweaking more esoteric parameters such as waveforms, feedback, and any sort of polyphony
- connect a better ADC for doing 1v/o properly

