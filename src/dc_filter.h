
#ifndef DC_FILTER_H
#define DC_FILTER_H

#include <stdint.h>

struct dc_filter_t
{
    int16_t prev_y_16;
    int16_t prev_x_16;
    int16_t alpha_16;
};


void dc_filter_init(dc_filter_t *pstate, float fcut_hz, float sample_rate_hz);
void dc_filter(dc_filter_t *pstate, int16_t *px_16, int num_samps, int16_t *py_16);


#endif

