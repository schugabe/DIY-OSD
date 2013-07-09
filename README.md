DIY-OSD
=======

An updated version of the DIY OSD from http://www.rcgroups.com/forums/showthread.php?t=1473207

Includes support for SimpleOSD X2 from Flytron http://www.rcgroups.com/forums/showpost.php?p=24214276&postcount=2254

How-to
------
**Warning** The changes have been not tested a lot so use firmware at your own risk.

Currently the config is set to display the signal quality from an ezuhf rx. So setup your ezuhf rx to output signal quality on a pin and connect the signal wire to the PB0 Pin (depending on your setup you might need to connect the ground of the rx to the ground of the osd as well but watch out for ground loops!!!) For other rx types the values in the config.h must be adjusted. 

If you don't know where to connect the wire: in the repository is a [picture](https://github.com/schugabe/DIY-OSD/blob/master/pb0.jpg "pb0") with a sign where PB0 is located. PC2 would be the pin for analog rssi readings.

Update the OSD Firmware:
* Download the code and open DIY_OSD_v0_18.ino in the Arduino editor (download from http://arduino.cc/en/Main/Software_ )
* Select the com port of the FTDI cable in the Tools/Serial Port Menu
* Select Arduino Pro or Pro Mini 5V,16Mhz with Atmega328 in the Tools/Board Menu as target device,
* Connect the FTDI cable to the usb port of the computer
* Wait for the COM port to show up and select the according device in the serial port menu
* Press the upload button
* Connect the FTDI cable to SimpleOSD X2. If there is a error message you have to try again. Disconnect the cable from the osd, click upload and connect again (you have to be fast but not too fast, it might take several tries)


Change-log
---------
* fixed a bug where wrong pin for config button was read on SimpleOSD X2
* changed analog rssi input to PC2 Pin (ADC or AN2 on schematic) for SimpleOSD X2
* added support for digital pwm rssi input on PIN PB0 for SimpleOSD X2
