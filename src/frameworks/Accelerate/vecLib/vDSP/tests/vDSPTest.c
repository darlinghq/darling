// Native test: compile with the vDSP sources it covers and run on the host.
//   cc -I include tests/vDSPTest.c src/arith.c src/convert.c src/fft.c src/extrema.c -lm -o vDSPTest
// Expected values follow the formulas in Apple's vDSP documentation.
#include <vDSP/vDSP.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int g_failures;

static void expectNear(double actual, double expected, double tolerance, const char* what, int index)
{
	if (fabs(actual - expected) > tolerance)
	{
		printf("FAIL %s[%d]: got %g, expected %g\n", what, index, actual, expected);
		g_failures++;
	}
}

static void expectFloats(const float* actual, const float* expected, int n, const char* what)
{
	for (int i = 0; i < n; i++)
		expectNear(actual[i], expected[i], 1e-5, what, i);
}

static void testArithmetic(void)
{
	const float a[] = { 1, 2, 3, 4 }, b[] = { 10, 20, 30, 40 };
	float c[4];
	float scalar = 2, offset = 100;

	vDSP_vadd(a, 1, b, 1, c, 1, 4);
	expectFloats(c, (float[]) { 11, 22, 33, 44 }, 4, "vadd");

	// A - B, with B passed first.
	vDSP_vsub(a, 1, b, 1, c, 1, 4);
	expectFloats(c, (float[]) { 9, 18, 27, 36 }, 4, "vsub");

	vDSP_vmul(a, 1, b, 1, c, 1, 4);
	expectFloats(c, (float[]) { 10, 40, 90, 160 }, 4, "vmul");

	vDSP_vsmul(a, 1, &scalar, c, 1, 4);
	expectFloats(c, (float[]) { 2, 4, 6, 8 }, 4, "vsmul");

	vDSP_vsadd(a, 1, &scalar, c, 1, 4);
	expectFloats(c, (float[]) { 3, 4, 5, 6 }, 4, "vsadd");

	vDSP_vsma(a, 1, &scalar, b, 1, c, 1, 4);
	expectFloats(c, (float[]) { 12, 24, 36, 48 }, 4, "vsma");

	float sum;
	vDSP_svesq(a, 1, &sum, 4);
	expectNear(sum, 30, 1e-5, "svesq", 0);

	float low = 1.5f, high = 3.5f;
	vDSP_vclip(a, 1, &low, &high, c, 1, 4);
	expectFloats(c, (float[]) { 1.5f, 2, 3, 3.5f }, 4, "vclip");

	float maximum;
	const float mixed[] = { 1, -7, 3 };
	vDSP_maxmgv(mixed, 1, &maximum, 3);
	expectNear(maximum, 7, 1e-5, "maxmgv", 0);

	// Strides, with every other element of the input and every other of the output.
	float wide[8] = { 0 };
	vDSP_vsadd(a, 2, &offset, wide, 2, 2);
	expectFloats(wide, (float[]) { 101, 0, 103, 0 }, 4, "vsadd stride");

	// In place.
	float in[] = { 1, 2 };
	vDSP_vmul(in, 1, in, 1, in, 1, 2);
	expectFloats(in, (float[]) { 1, 4 }, 2, "vmul in place");
}

static void testConvolution(void)
{
	const float signal[] = { 1, 2, 3, 4, 5 };
	const float filter[] = { 1, 10 };
	float out[4];

	// Positive stride: correlation.
	vDSP_conv(signal, 1, filter, 1, out, 1, 4, 2);
	expectFloats(out, (float[]) { 21, 32, 43, 54 }, 4, "conv correlation");

	// Negative stride from the filter's end: convolution.
	vDSP_conv(signal, 1, filter + 1, -1, out, 1, 4, 2);
	expectFloats(out, (float[]) { 12, 23, 34, 45 }, 4, "conv convolution");
}

static void testDeq22(void)
{
	const float in[] = { 0, 0, 1, 0, 0, 0 };
	const float coefficients[] = { 1, 0.5f, 0.25f, -0.5f, 0.1f };
	float out[6] = { 0, 0 };

	// The impulse response, worked out by hand from the difference equation.
	vDSP_deq22(in, 1, coefficients, out, 1, 4);
	expectFloats(out, (float[]) { 0, 0, 1, 1, 0.65f, 0.225f }, 6, "deq22");

	const double inD[] = { 0, 0, 1, 0, 0, 0 };
	const double coefficientsD[] = { 1, 0.5, 0.25, -0.5, 0.1 };
	double outD[6] = { 0, 0 };
	vDSP_deq22D(inD, 1, coefficientsD, outD, 1, 4);
	expectNear(outD[5], 0.225, 1e-12, "deq22D", 5);
}

static void testComplex(void)
{
	const DSPComplex packed[] = { { 1, 2 }, { 3, 4 }, { 5, 6 } };
	float re[3], im[3];
	DSPSplitComplex split = { re, im };

	// The stride of a DSPComplex vector counts floats.
	vDSP_ctoz(packed, 2, &split, 1, 3);
	expectFloats(re, (float[]) { 1, 3, 5 }, 3, "ctoz real");
	expectFloats(im, (float[]) { 2, 4, 6 }, 3, "ctoz imag");

	DSPComplex back[3];
	vDSP_ztoc(&split, 1, back, 2, 3);
	expectFloats(&back[0].real, &packed[0].real, 6, "ztoc");

	float ar[] = { 1 }, ai[] = { 2 }, br[] = { 3 }, bi[] = { 4 }, cr[1], ci[1];
	DSPSplitComplex A = { ar, ai }, B = { br, bi }, C = { cr, ci };

	vDSP_zvmul(&A, 1, &B, 1, &C, 1, 1, 1);
	expectNear(cr[0], -5, 1e-5, "zvmul real", 0);
	expectNear(ci[0], 10, 1e-5, "zvmul imag", 0);

	vDSP_zvmul(&A, 1, &B, 1, &C, 1, 1, -1);
	expectNear(cr[0], 11, 1e-5, "zvmul conj real", 0);
	expectNear(ci[0], -2, 1e-5, "zvmul conj imag", 0);
}

// Compares with a directly evaluated DFT, at a smaller length than the setup's as well.
static void testFFT(vDSP_Length setupLog2, vDSP_Length log2n, vDSP_Stride stride)
{
	const int n = 1 << log2n, m = n / 2;
	FFTSetup setup = vDSP_create_fftsetup(setupLog2, kFFTRadix2);
	if (!setup)
	{
		printf("FAIL: no FFT setup\n");
		g_failures++;
		return;
	}

	double* signal = malloc(n * sizeof(double));
	float* re = calloc(m * stride, sizeof(float));
	float* im = calloc(m * stride, sizeof(float));
	DSPSplitComplex z = { re, im };

	srand(1234 + (unsigned) log2n);
	for (int i = 0; i < n; i++)
		signal[i] = rand() / (double) RAND_MAX - 0.5;
	for (int i = 0; i < m; i++)
	{
		re[i * stride] = (float) signal[2 * i];
		im[i * stride] = (float) signal[2 * i + 1];
	}

	vDSP_fft_zrip(setup, &z, stride, log2n, kFFTDirection_Forward);

	const double tolerance = 2e-5 * n;
	for (int k = 0; k <= m; k++)
	{
		double dftR = 0, dftI = 0;
		for (int i = 0; i < n; i++)
		{
			const double angle = -2.0 * M_PI * k * i / n;
			dftR += signal[i] * cos(angle);
			dftI += signal[i] * sin(angle);
		}
		// Twice the mathematical DFT, with the real-only bin N/2 stored in imagp[0].
		if (k == 0)
			expectNear(re[0], 2 * dftR, tolerance, "fft dc", k);
		else if (k == m)
			expectNear(im[0], 2 * dftR, tolerance, "fft nyquist", k);
		else
		{
			expectNear(re[k * stride], 2 * dftR, tolerance, "fft real", k);
			expectNear(im[k * stride], 2 * dftI, tolerance, "fft imag", k);
		}
	}

	vDSP_fft_zrip(setup, &z, stride, log2n, kFFTDirection_Inverse);

	// A round trip multiplies by 2N.
	for (int i = 0; i < m; i++)
	{
		expectNear(re[i * stride] / (2.0 * n), signal[2 * i], 1e-4, "fft round trip even", i);
		expectNear(im[i * stride] / (2.0 * n), signal[2 * i + 1], 1e-4, "fft round trip odd", i);
	}

	free(signal);
	free(re);
	free(im);
	vDSP_destroy_fftsetup(setup);
}

int main(void)
{
	testArithmetic();
	testConvolution();
	testDeq22();
	testComplex();

	for (vDSP_Length log2n = 1; log2n <= 12; log2n++)
		testFFT(log2n, log2n, 1);
	testFFT(12, 8, 1);
	testFFT(10, 10, 3);

	if (vDSP_create_fftsetup(4, kFFTRadix3))
	{
		printf("FAIL: radix 3 should be refused\n");
		g_failures++;
	}
	vDSP_destroy_fftsetup(NULL);

	if (g_failures)
	{
		printf("vDSPTest: %d failures\n", g_failures);
		return 1;
	}
	printf("vDSPTest: OK\n");
	return 0;
}
