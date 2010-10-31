#ifndef __RTC_H__
#define __RTC_H__

#include "defs.h"

struct rtc
{
	int batt;
	int sel;
	int latch;
	int d, h, m, s, t;
	int stop, carry;
	byte regs[8];
};

extern struct rtc rtc;

void rtc_latch(byte b);
void rtc_write(byte b);

#endif



