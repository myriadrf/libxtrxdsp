#include <xtrxdsp_fft.h>
#include <math.h>

static unsigned char reverse(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

int xtrxdsp_fft_realign_pwr_d(const uint16_t* samples,
							  unsigned fftbins,
							  double scale,
							  double* outv)
{
	if (fftbins != 512)
		return -EINVAL;

	double mulc = scale * 10.0 / 1024 / log2(10);

	for (unsigned i = 0; i < 512; i += 4) {
		uint16_t v[4];
		unsigned idx = reverse(i >> 1);
		v[0] = samples[idx];
		v[1] = samples[idx + 256];
		v[2] = samples[idx + 128];
		v[3] = samples[idx + 384];

		int q[4];
		q[0] = ((int)v[0]) - 65535 + 8192;
		q[1] = ((int)v[1]) - 65535 + 8192;
		q[2] = ((int)v[2]) - 65535 + 8192;
		q[3] = ((int)v[3]) - 65535 + 8192;

		double vv[4];
		vv[0] = mulc * ((double)q[0]);
		vv[1] = mulc * ((double)q[1]);
		vv[2] = mulc * ((double)q[2]);
		vv[3] = mulc * ((double)q[3]);

		unsigned j = i ^ 256;
		outv[j+0] = vv[0];
		outv[j+1] = vv[1];
		outv[j+2] = vv[2];
		outv[j+3] = vv[3];
	}

	return 0;
}
