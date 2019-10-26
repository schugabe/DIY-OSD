DIY-OSD
=======

This firmware is an updated version of the DIY OSD firmware ( http://www.rcgroups.com/forums/showthread.php?t=1473207 ) that includes support for the SimpleOSD X2 from Flytron.

Beside of general improvements and small bug fixes this firmware supports digital rssi indication. Additionally a software low pass filter has been added to smooth the current/voltage readings.

How-to
------
**Warning** Use this firmware at your own risk.

The example config is set to display the signal quality from an ezuhf rx. It is necessary to configure the ezuhf rx to output the signal quality on a pin and connect the signal wire to the PB0 Pin. Depending on the setup it might be necessary to connect the ground wire of the rx to the ground pin of the OSD (watch out for ground loops!). For other rx types the values rssi_min and rssi_max found in config.h must be adjusted. To find out the necessary values it is possible to set show_raw_rssi. If show_raw_rssi is set to 1 the unmodified duration of the PWM signal is displayed as RSSI. The value of rssi_max is the value visible with the tx switched on and rssi_min is the value visible where fail-safe is activated.

In the folder MeasureRSSI is a sketch that measures the digital RSSI signal and sends information to the serial interface.

In this the location of PB0 is shown:

![pb0 location](https://github.com/schugabe/DIY-OSD/blob/master/pb0.jpg) 

Update the OSD Firmware:
* Download the code and open DIY_OSD.ino in the Arduino editor (download from http://arduino.cc/en/Main/Software_ )
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
* added low pass filter to smooth voltage/current reading
