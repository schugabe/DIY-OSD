//#include "variables.h"

#ifndef output_h
#define output_h

void detectframe();
void detectline();

#if (digital_rssi==1)
void pwm_enable();
#endif
#endif
