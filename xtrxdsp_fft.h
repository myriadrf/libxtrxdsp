#include <xtrxdsp.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

int xtrxdsp_fft_realign_pwr_d(const uint16_t* samples,
							  unsigned fftbins,
							  double scale,
							  double* outv);


#ifdef __cplusplus
}
#endif
