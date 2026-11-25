
This document is so you (the engineer) can quickly setup and use RMS GUI to begin flashing the CM200dx inveter.

1. Download RMS GUI from this link: 
https://www.cascadiamotion.com/documentation
Look for RMS GUI and GTK environment download

2. Once both softwares are downloaded you will need to connect RS-232 to the inverter then connect the USB to your PC. Once connected open RMS GUI.

3. The default “def syms” file will be opened. You will want to load a different one related to our motor, so you should load PM_Motor_defsyms. Since our motor is custom. 

4. Every-time you load a different defsyms file, you have to power down the inverter then power it back on. Power cycling is the term

5. Once the new defsymms is loaded, you can write values to EEPROM, once written click program.

## You now have the ability to flash our current inverter.

