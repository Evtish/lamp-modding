#include "rtc.h"

#define MONTH_REGISTER 0x05

// converts datetime from BCD to decimal (size of formatted_dt must be equal to 3 * bcd_dt_size)
void rtc_format_datetime(char *formatted_dt, const uint8_t *bcd_dt, const uint8_t bcd_dt_size) {
    for (uint8_t i = 0; i < bcd_dt_size; i++) {
        uint8_t fst_digit = (bcd_dt[i] >> 4);
        if (i == MONTH_REGISTER) fst_digit &= 0x07; // the MSB of month register is century, so we need to mask it
        
        uint8_t snd_digit = (bcd_dt[i] & 0x0F);
        
        char sep = ' ';
        if (i >= bcd_dt_size - 1) sep = '\0';

        formatted_dt[i * 3]     = '0' + fst_digit;
        formatted_dt[i * 3 + 1] = '0' + snd_digit;
        formatted_dt[i * 3 + 2] = sep;
    }
}
