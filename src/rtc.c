#include "twi.c"
#include "rtc.h"
#include "utils.h"

#define MONTH_REGISTER 0x05

// TODO: init all bits in all registers with defaults values (e.g. 12/24 bit in the hours register)
uint8_t rtc_init(void) {
	const uint8_t register_state[] = {
		// Clock
		0,	// seconds BCD
		0,	// minutes BCD
		18 & ~(1 << 6),	// hours BCD, 12/24 bit
		// Calendar
		3,	// day BCD
		49,	// date BCD
		18, // month BCD
		8229,	// year BCD
		// Alarm 1
		0 & ~(1 << 7), // seconds BCD, A1M1 bit
		0 & ~(1 << 7), // minutes BCD, A1M2 bit
		9 & ~(1 << 7) & ~(1 << 6), // hours BCD, A1M3 bit & 12/24 bit
		// Alarm 2
		(1 << 7), // A1M4 bit
		0 & ~(1 << 7), // minutes BCD, A1M2 bit
		33 & ~(1 << 7) & ~(1 << 6), // hours BCD, A1M3 bit & 12/24 bit
		(1 << 7), // A1M4 bit
		// Control
		(1 << 2) | (1 << 1) | (1 << 0), // INTCN, A2IE, A1IE
	};

	return twi_transmit_bytes(register_state, 0x00, array_len(register_state));
}

// converts datetime from BCD to decimal (size of formatted_dt must be equal to 3 * bcd_dt_size)
void rtc_format_datetime(char *formatted_dt, const uint8_t *bcd_dt, const uint8_t bcd_dt_size) {
	for (uint8_t i = 0; i < bcd_dt_size; i++) {
		uint8_t fst_digit = (bcd_dt[i] >> 4);
		if (i == MONTH_REGISTER) fst_digit &= 0x07; // the MSB of month register is century, so we need to mask it
		
		uint8_t snd_digit = (bcd_dt[i] & 0x0F);
		
		char sep = ' ';
		if (i >= bcd_dt_size - 1) sep = '\0';

		formatted_dt[i * 3]		= '0' + fst_digit;
		formatted_dt[i * 3 + 1] = '0' + snd_digit;
		formatted_dt[i * 3 + 2] = sep;
	}
}
