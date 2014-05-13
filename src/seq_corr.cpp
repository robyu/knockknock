#include "seq_corr.h"
#include "utils.h"
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <float.h>

struct seq_stats_t
{
    int len;
    float max;
    float var;
    float mean;
};

seq_stats_t target_seq_stats;

uint32_t ptarget_seq_ms[SC_NUM_EVENTS_TWO_BITS] = {370,200,200,370};


//
// given sequence x[1..len_x], return
// sum(x) 
// max x
// sum 
static void get_stats(uint32_t *px, int len_x, seq_stats_t *pstats)
{
    int n;
    float mean;
    float max = -FLT_MAX;
    float var;


    pstats->len = len_x;

    //
    // mean and max
    // 
    // E[x] = (1/N) * sum(xi)
    //
    mean = 0.0f;
    for (n=0;n<len_x;n++)
    {
        float xflt= (float)px[n];   
        //utils_log("mean calc: x[%d] = %d\n",n,px[n]);
        mean += xflt;
        if (xflt > max)
        {
            max = xflt;
        }
    }
    mean = mean / (float)len_x;
    // utils_log("mean=");
    // utils_print_float(mean,2);
    // utils_log("\n");

    //
    // variance
    // = (1/N)*sum ((xi - E[x])^2)
    var = 0.0f;
    for (n=0;n<len_x;n++)
    {
        float xflt= (float)px[n];
        var += (xflt -mean) * (xflt-mean);
        //utils_log("[%d] [%d]\n",n,px[n]);
        //var = var + (xflt - 285.0);
        // utils_print_float(var,2);
        // utils_log("\n");
    }
    var = var / (float)len_x;
    
    // utils_log("var=");
    // utils_print_float(var,8);
    // utils_log("\n");

    pstats->max = max;
    pstats->var = var;
    pstats->mean = mean;
}

//
// compute up-front statistics 
// on target sequence
void seq_corr_init(void)
{
    //
    // compute mean and find max
    get_stats(ptarget_seq_ms, SC_NUM_EVENTS_TWO_BITS, &target_seq_stats);
}

//
// get squared correlation coeff (rho^2)
float get_r2(uint32_t *py, seq_stats_t *pstats_y,
                       uint32_t *px, seq_stats_t *pstats_x)
{
    float cov_xy = 0.0f;
    float r2;
    int n;
    UTILS_ASSERT(pstats_x->len == pstats_y->len);

    //
    // compute covariance = sum( (xi-mean(x)) (yi - mean(y)))
    for (n=0;n<pstats_x->len;n++)
    {
        float xterm;
        float yterm;
        xterm = (px[n] - pstats_x->mean);
        yterm = (py[n] - pstats_y->mean);
        cov_xy += xterm * yterm;
    }
    cov_xy = cov_xy / (float)pstats_x->len;

    r2 = (cov_xy * cov_xy) / (pstats_x->var * pstats_y->var);
    utils_log("r2=");
    utils_print_float(r2,4);
    utils_log("\n");
    return r2;
}



//
// compute correlation coefficient, decide whether we have match
int seq_corr_detect(uint32_t *py, int len_y)
{
    seq_stats_t stats_y;
    float r_sqrd;
    int detect;

    UTILS_ASSERT(len_y >= SC_NUM_EVENTS_TWO_BITS);  // input seq length must equal or exceed target seq length
    
    // compute stats only for first N events
    get_stats(py, SC_NUM_EVENTS_TWO_BITS, &stats_y);

    r_sqrd = get_r2(py, &stats_y, ptarget_seq_ms, &target_seq_stats);

    if (r_sqrd > SC_DETECT_THRESHOLD)
    {
        utils_log("positive detect\n");
        detect = 1;
    }
    else
    {
        utils_log("negative detect\n");
        detect = 0;
    }
    return detect;
}
