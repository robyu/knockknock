#include "dc_filter.h"
#include <string.h>
#include <stdint.h>
#include "utils.h"

/*
  one-pole highpass filter
   y[n] = alpha * (y[n-1] + x[n] - x[n-1])

   fcut_hz corresponds to 3 dB point
*/
void dc_filter_init(dc_filter_t *pstate, float fcut_hz, float sample_rate_hz)
{
    float temp;

    memset(pstate, 0, sizeof(dc_filter_t));
    //
    // alpha = 32767 - fcut * 26 * 8 / samplerate_khz
    //
    // warning:  if alpha is too large, then lack of precision in 
    // q15 * i16 multiply will
    // result in sticky bits
    //
    temp = 26.0f * 8.0f / (sample_rate_hz/1000.0f);
    temp = fcut_hz * temp;

    pstate->alpha_16 = (int16_t)32767 - (int16_t)temp;
    utils_log("/ndc_filter: alpha_i16 = %d\n",pstate->alpha_16);
    if (pstate->alpha_16 <= 0)
    {
        utils_log("fcut_hz = ");
        utils_print_float(fcut_hz,5);
        utils_log("\n");
        utils_log("alpha must be > 0");
        UTILS_ASSERT(0);
    }
}

#if 0
int32 mul_q15_i32(int16_t q15, int32_t y32)
{

    int32_t result32;
    int32_t upper32;
    int32_t lower32;
    int16_t y16;
    uint16_t uy16;
    int32_t round32 = ((int32)1)<<14;
    int resultIsPositive;

    resultIsPositive = ((q15>=0) && (y32>=0)) || ((q15<0) && (y32<0));

    //
    // break up computation into 2 16bit x 16bit partial products,
    // then combine partial products


    //
    // upper partial product
    y16 = (int16)(y32>>16);                      // LO(s(31,0) >> 16) = s(31,-16)
    upper32 = (int32)q15 * (int32)y16;          // s(0,15) * s(31,-16) = s(32,-1)
    upper32 <<= 1;                              // s(32,-1) << 1 = s(31,0)
        
    // lower partial product 
    uy16 = (uint16)(y32 & (uint32)0x0000ffff);  // LO(s(31,0)) = s(15,0)
    // assign to unsigned type to void sign extension
    lower32 = (int32)q15 * (int32)uy16;         // s(0,15) * s(15,0) = s(16,15)
    // round and truncate lower partial product
    lower32 += round32;
    lower32 = lower32>>15;                    // s(16,15) >> 15 = s(31,0)

    // combine partial products
    result32 = upper32 + lower32;

    // saturate if positive
    if ((resultIsPositive) && ((uint32)result32 > SE_INT_MAX32))
    {
        result32 = SE_INT_MAX32;
    }

	return result32;
}
#endif

//
// multiply 16-bit integer against q15 value
int16_t mul_q15_i16_i16(int16_t a_q15, int16_t b16)
{
    int32_t c32;

    c32 = (int32_t)a_q15 * (int32_t)b16;

    // round and truncate
    c32 += (int32_t)(1<<14);
    c32 = c32 >> 15;

    //
    // don't worry about saturation, etc for now
    return (int16_t)(0xffff & (uint32_t)c32);
}

//
// implements a dc_filter
// because the filter maintains 16-bit states, the output is suscptible to
// sticky bits--the output hovers at +/- 2.
// avoid this by choosing sufficiently large fcutoff s.t. alpha is sufficiently large
// e.g. 16384/32767
void dc_filter(dc_filter_t *pstate, int16_t *px_16, int num_samps, int16_t *py_16)
{
    int n;
    int16_t prev_y_16 = pstate->prev_y_16;
    int16_t prev_x_16 = pstate->prev_x_16;


    for (n=0;n<num_samps;n++)
    {
        int16_t x_16;
        int16_t temp_16;
        int16_t y_16;

        // y = alpha * ( y(n-1) + x(n) - x(n-1) )
        x_16 = *px_16;
        temp_16 = prev_y_16 + (x_16 - prev_x_16);

        y_16 = mul_q15_i16_i16(pstate->alpha_16, temp_16);
        
        *py_16 = y_16;  // write output

        // update state
        prev_x_16 = x_16;
        prev_y_16 = y_16; 

        px_16++;
        py_16++;
    }

    //
    // save state
    pstate->prev_x_16 = prev_x_16;
    pstate->prev_y_16 = prev_y_16;
}
