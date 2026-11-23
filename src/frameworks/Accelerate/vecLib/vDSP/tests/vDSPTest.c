// Native test: compile with the vDSP sources it covers and run on the host.
//   cc -I include tests/vDSPTest.c src/arith.c src/convert.c src/fft.c src/extrema.c src/reduce.c -lm -o vDSPTest
// Expected values follow the formulas in Apple's vDSP documentation.
#include <vDSP/vDSP.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int g_failures;

static void expectNear(double actual, double expected, double tolerance, const char* what, int index)
{
	// A NaN must match a NaN: fabs(NaN - x) > tolerance is false, so it would otherwise pass for any expected value.
	const int nanMismatch = isnan(actual) != isnan(expected);
	if (nanMismatch || (!isnan(actual) && fabs(actual - expected) > tolerance))
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

// Every case below is one of the examples on the vDSP function pages, or a hand-worked variation of one:
// https://developer.apple.com/documentation/accelerate/vdsp_vdiv and the sibling pages named per case.
static void testRealVectorOps(void)
{
	float c[8];

	// vDSP_vdiv: B is passed first, the result is A / B (a = 10..50, b = 1..5 gives 10 everywhere).
	const float divA[] = { 10, 20, 30, 40, 50 }, divB[] = { 1, 2, 3, 4, 5 };
	vDSP_vdiv(divB, 1, divA, 1, c, 1, 5);
	expectFloats(c, (float[]) { 10, 10, 10, 10, 10 }, 5, "vdiv");

	const double divAD[] = { 10, 20, 30 }, divBD[] = { 1, 2, 3 };
	double cD[3];
	vDSP_vdivD(divBD, 1, divAD, 1, cD, 1, 3);
	expectNear(cD[2], 10, 1e-12, "vdivD", 2);

	// vDSP_vsdiv: C[n] = A[n] / B.
	const float sA[] = { 1, 2, 3, 4, 5 };
	float ten = 10;
	vDSP_vsdiv(sA, 1, &ten, c, 1, 5);
	expectFloats(c, (float[]) { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f }, 5, "vsdiv");

	// vDSP_vneg, vDSP_vabs and vDSP_vnabs share the input of their pages.
	const float values[] = { -1, 2, -3, 4, -5, 6, -7, 8 };
	vDSP_vneg(values, 1, c, 1, 8);
	expectFloats(c, (float[]) { 1, -2, 3, -4, 5, -6, 7, -8 }, 8, "vneg");
	vDSP_vabs(values, 1, c, 1, 8);
	expectFloats(c, (float[]) { 1, 2, 3, 4, 5, 6, 7, 8 }, 8, "vabs");
	vDSP_vnabs(values, 1, c, 1, 8);
	expectFloats(c, (float[]) { -1, -2, -3, -4, -5, -6, -7, -8 }, 8, "vnabs");

	const double valuesD[] = { -1.5, 2.5 };
	vDSP_vnegD(valuesD, 1, cD, 1, 2);
	expectNear(cD[0], 1.5, 1e-12, "vnegD", 0);
	vDSP_vabsD(valuesD, 1, cD, 1, 2);
	expectNear(cD[0], 1.5, 1e-12, "vabsD", 0);
	vDSP_vnabsD(valuesD, 1, cD, 1, 2);
	expectNear(cD[1], -2.5, 1e-12, "vnabsD", 1);

	// vDSP_vsq: the page example.
	const float sq[] = { -2, -0.5f, 0.5f, 3 };
	vDSP_vsq(sq, 1, c, 1, 4);
	expectFloats(c, (float[]) { 4, 0.25f, 0.25f, 9 }, 4, "vsq");

	// vDSP_vma: D = A * B + C. vDSP_vmsa: D = A * B + scalar.
	const float ma[] = { 1, 2, 3, 4, 5 }, mb[] = { 10, 20, 30, 40, 50 }, mc[] = { 5, 4, 3, 2, 1 };
	vDSP_vma(ma, 1, mb, 1, mc, 1, c, 1, 5);
	expectFloats(c, (float[]) { 15, 44, 93, 162, 251 }, 5, "vma");
	float five = 5;
	vDSP_vmsa(ma, 1, mb, 1, &five, c, 1, 5);
	expectFloats(c, (float[]) { 15, 45, 95, 165, 255 }, 5, "vmsa");
	const double maD[] = { 2 }, mbD[] = { 3 }, mcD[] = { 4 };
	vDSP_vmaD(maD, 1, mbD, 1, mcD, 1, cD, 1, 1);
	expectNear(cD[0], 10, 1e-12, "vmaD", 0);
	vDSP_vmsaD(maD, 1, mbD, 1, mcD, cD, 1, 1);
	expectNear(cD[0], 10, 1e-12, "vmsaD", 0);

	// vDSP_vmax, vDSP_vmin and vDSP_vthr: the page example, a = {-2, -0.5, 0.5, 3}.
	const float twos[] = { 2, 2, 2, 2 };
	vDSP_vmax(sq, 1, twos, 1, c, 1, 4);
	expectFloats(c, (float[]) { 2, 2, 2, 3 }, 4, "vmax");
	vDSP_vmin(sq, 1, twos, 1, c, 1, 4);
	expectFloats(c, (float[]) { -2, -0.5f, 0.5f, 2 }, 4, "vmin");
	float threshold = -1;
	vDSP_vthr(sq, 1, &threshold, c, 1, 4);
	expectFloats(c, (float[]) { -1, -0.5f, 0.5f, 3 }, 4, "vthr");

	// Ties between -0 and +0 pin the comparisons in the page loops: vmax takes A when B <= A, vmin takes A
	// when A <= B, vthr takes A when A >= threshold.
	const float negZero = -0.0f, posZero = 0.0f;
	vDSP_vmax(&negZero, 1, &posZero, 1, c, 1, 1);
	expectNear(signbit(c[0]) ? 1 : 0, 1, 0, "vmax tie takes A", 0);
	vDSP_vmin(&posZero, 1, &negZero, 1, c, 1, 1);
	expectNear(signbit(c[0]) ? 1 : 0, 0, 0, "vmin tie takes A", 0);
	vDSP_vthr(&negZero, 1, &posZero, c, 1, 1);
	expectNear(signbit(c[0]) ? 1 : 0, 1, 0, "vthr tie takes A", 0);

	// QUIRK test (see vDSP_vthr in arith.c): a NaN element passes through.
	const float nanValue = NAN;
	vDSP_vthr(&nanValue, 1, &threshold, c, 1, 1);
	expectNear(c[0], NAN, 0, "vthr quirk NaN passes through", 0);

	// Double precision: a -0 / +0 tie pins vmaxD (takes A), vminD (takes A) and vthrD (takes A); vsqD squares.
	const double negZeroD = -0.0, posZeroD = 0.0, minusTwo = -2;
	double d;
	vDSP_vmaxD(&negZeroD, 1, &posZeroD, 1, &d, 1, 1);
	expectNear(signbit(d) ? 1 : 0, 1, 0, "vmaxD tie takes A", 0);
	vDSP_vminD(&posZeroD, 1, &negZeroD, 1, &d, 1, 1);
	expectNear(signbit(d) ? 1 : 0, 0, 0, "vminD tie takes A", 0);
	vDSP_vthrD(&negZeroD, 1, &posZeroD, &d, 1, 1);
	expectNear(signbit(d) ? 1 : 0, 1, 0, "vthrD tie takes A", 0);
	vDSP_vmaxD(&minusTwo, 1, &negZeroD, 1, &d, 1, 1);
	expectNear(d, 0, 0, "vmaxD picks the larger", 0);
	vDSP_vminD(&minusTwo, 1, &negZeroD, 1, &d, 1, 1);
	expectNear(d, -2, 0, "vminD picks the smaller", 0);
	vDSP_vsqD(&minusTwo, 1, &d, 1, 1);
	expectNear(d, 4, 0, "vsqD", 0);

	// vDSP_vfill, vDSP_vclr and vDSP_vramp.
	float pi = (float) M_PI;
	float filled[10];
	vDSP_vfill(&pi, filled, 1, 10);
	expectNear(filled[0], pi, 0, "vfill", 0);
	expectNear(filled[9], pi, 0, "vfill", 9);
	float cleared[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	vDSP_vclr(cleared, 1, 10);
	for (int i = 0; i < 10; i++)
		expectNear(cleared[i], 0, 0, "vclr", i);
	float start = 0, step = 1;
	vDSP_vramp(&start, &step, c, 1, 8);
	expectFloats(c, (float[]) { 0, 1, 2, 3, 4, 5, 6, 7 }, 8, "vramp");
	double startD = 1, stepD = -0.5, rampD[3];
	vDSP_vrampD(&startD, &stepD, rampD, 1, 3);
	expectNear(rampD[2], 0, 1e-12, "vrampD", 2);

	// vDSP_vrvrs: the page example, then an odd length.
	float reversed[10] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
	vDSP_vrvrs(reversed, 1, 10);
	expectFloats(reversed, (float[]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, "vrvrs");
	float odd[] = { 1, 2, 3 };
	vDSP_vrvrs(odd, 1, 3);
	expectFloats(odd, (float[]) { 3, 2, 1 }, 3, "vrvrs odd");

	// A stride of 2 touches only every other element, in the input and the output alike.
	float strided[] = { 1, 99, 2, 99, 3, 99 };
	vDSP_vrvrs(strided, 2, 3);
	expectFloats(strided, (float[]) { 3, 99, 2, 99, 1, 99 }, 6, "vrvrs stride");
	float wide[6] = { 0, 77, 0, 77, 0, 77 };
	vDSP_vneg(strided, 2, wide, 2, 3);
	expectFloats(wide, (float[]) { -3, 77, -2, 77, -1, 77 }, 6, "vneg stride");

	// A count of zero writes nothing.
	float untouched[] = { 42 };
	vDSP_vfill(&pi, untouched, 1, 0);
	vDSP_vrvrs(untouched, 1, 0);
	expectNear(untouched[0], 42, 0, "zero count", 0);
}

// The inputs and expected values are those on the pages of the vDSP reduction functions, for example
// https://developer.apple.com/documentation/accelerate/vdsp_sve and vdsp_dotpr.
static void testReductions(void)
{
	const float a[] = { -1.5f, 2.25f, 3.6f, 0.2f, -0.1f, -4.3f };
	float c;

	vDSP_sve(a, 1, &c, 6);
	expectNear(c, 0.15, 1e-5, "sve", 0);
	vDSP_svemg(a, 1, &c, 6);
	expectNear(c, 11.95, 1e-5, "svemg", 0);
	vDSP_svs(a, 1, &c, 6);
	expectNear(c, -2.6875, 1e-4, "svs", 0);
	vDSP_maxv(a, 1, &c, 6);
	expectNear(c, 3.6, 1e-6, "maxv", 0);
	vDSP_minv(a, 1, &c, 6);
	expectNear(c, -4.3, 1e-6, "minv", 0);

	// An empty maximum is -infinity and an empty minimum is infinity, per the pages.
	c = 0;
	vDSP_maxv(a, 1, &c, 0);
	if (!isinf(c) || c > 0)
	{
		printf("FAIL maxv empty: got %g\n", c);
		g_failures++;
	}
	vDSP_minv(a, 1, &c, 0);
	if (!isinf(c) || c < 0)
	{
		printf("FAIL minv empty: got %g\n", c);
		g_failures++;
	}

	const float stats[] = { -8, -4, -2, 0, 2, 4, 8 };
	vDSP_meanv(stats, 1, &c, 7);
	expectNear(c, 0, 1e-6, "meanv", 0);
	vDSP_measqv(stats, 1, &c, 7);
	expectNear(c, 24, 1e-5, "measqv", 0);
	vDSP_rmsqv(stats, 1, &c, 7);
	expectNear(c, 4.899, 1e-3, "rmsqv", 0);

	// vDSP_mvessq page example: the mean of signed squares of this vector is 0, where measqv above gives 24.
	// https://developer.apple.com/documentation/accelerate/vdsp_mvessq
	vDSP_mvessq(stats, 1, &c, 7);
	expectNear(c, 0, 1e-6, "mvessq", 0);
	const float skew[] = { 1, -2, 3, 4 };
	// (1 - 4 + 9 + 16) / 4, with every other element when the stride is 2: (1 + 9) / 2.
	vDSP_mvessq(skew, 1, &c, 4);
	expectNear(c, 5.5, 1e-6, "mvessq signed", 0);
	vDSP_mvessq(skew, 2, &c, 2);
	expectNear(c, 5, 1e-6, "mvessq stride", 0);
	// A negative stride walks backwards from the pointer given (macOS 26.7.3 and 10.14.4 agree).
	vDSP_mvessq(skew + 3, -1, &c, 4);
	expectNear(c, 5.5, 1e-6, "mvessq negative stride", 0);
	// QUIRK test: N equal to zero is NaN on macOS 26.7.3 and 10.14.4, not the -infinity the vDSP_mvessqD page
	// states (see reduce.c).
	c = 0;
	vDSP_mvessq(skew, 1, &c, 0);
	if (!isnan(c))
	{
		printf("FAIL mvessq empty is NaN (quirk): got %g\n", c);
		g_failures++;
	}

	const float dotA[] = { 1, 2, 3, 4 }, dotB[] = { 5, 6, 7, 8 };
	vDSP_dotpr(dotA, 1, dotB, 1, &c, 4);
	expectNear(c, 70, 1e-5, "dotpr", 0);

	// Strides: every other element of each input.
	vDSP_sve(a, 2, &c, 3);
	expectNear(c, -1.5 + 3.6 - 0.1, 1e-5, "sve stride", 0);
	vDSP_dotpr(dotA, 2, dotB, 2, &c, 2);
	expectNear(c, 1 * 5 + 3 * 7, 1e-5, "dotpr stride", 0);
	vDSP_maxv(a, 3, &c, 2);
	expectNear(c, 0.2, 1e-6, "maxv stride", 0);

	const double d[] = { -1.5, 2.25, 3.6, 0.2, -0.1, -4.3 }, e[] = { 1, 2, 3, 4, 5, 6 };
	double r;
	vDSP_sveD(d, 1, &r, 6);
	expectNear(r, 0.15, 1e-12, "sveD", 0);
	vDSP_svemgD(d, 1, &r, 6);
	expectNear(r, 11.95, 1e-12, "svemgD", 0);
	vDSP_svsD(d, 1, &r, 6);
	expectNear(r, -2.6875, 1e-12, "svsD", 0);
	vDSP_maxvD(d, 1, &r, 6);
	expectNear(r, 3.6, 1e-12, "maxvD", 0);
	vDSP_minvD(d, 1, &r, 6);
	expectNear(r, -4.3, 1e-12, "minvD", 0);
	vDSP_meanvD(e, 1, &r, 6);
	expectNear(r, 3.5, 1e-12, "meanvD", 0);
	vDSP_measqvD(e, 1, &r, 6);
	expectNear(r, 91.0 / 6, 1e-12, "measqvD", 0);
	vDSP_rmsqvD(e, 1, &r, 6);
	expectNear(r, sqrt(91.0 / 6), 1e-12, "rmsqvD", 0);
	const double f[] = { 1, -2, 3, -4 };
	vDSP_mvessqD(f, 1, &r, 4);
	expectNear(r, (1 - 4 + 9 - 16) / 4.0, 1e-12, "mvessqD", 0);
	r = 0;
	vDSP_mvessqD(f, 1, &r, 0);
	if (!isnan(r))
	{
		printf("FAIL mvessqD empty is NaN (quirk): got %g\n", r);
		g_failures++;
	}
	vDSP_dotprD(e, 1, e, 1, &r, 6);
	expectNear(r, 91, 1e-12, "dotprD", 0);
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

// vDSP_maxvi, vDSP_minvi and the double-precision pair.
// https://developer.apple.com/documentation/accelerate/vdsp_maxvi
// The pages give the stride 1 example (max 3.6 index 2; min -4.3 index 5) and the empty result (C is -infinity or
// infinity, I is 0). The stride and tie cases follow the public vDSP.h (macOS 10.15 SDK) sentence quoted in
// extrema.c; the negative stride and NaN cases pin GUESS choices made there.
static void expectIndex(vDSP_Length actual, vDSP_Length expected, const char* what)
{
	if (actual != expected)
	{
		printf("FAIL %s: index %lu, expected %lu\n", what, (unsigned long) actual, (unsigned long) expected);
		g_failures++;
	}
}

static void testExtremaIndex(void)
{
	const float a[] = { -1.5f, 2.25f, 3.6f, 0.2f, -0.1f, -4.3f };
	const double d[] = { -1.5, 2.25, 3.6, 0.2, -0.1, -4.3 };
	float c;
	double r;
	vDSP_Length i;

	vDSP_maxvi(a, 1, &c, &i, 6);
	expectNear(c, 3.6, 1e-6, "maxvi", 0);
	expectIndex(i, 2, "maxvi");
	vDSP_minvi(a, 1, &c, &i, 6);
	expectNear(c, -4.3, 1e-6, "minvi", 0);
	expectIndex(i, 5, "minvi");
	vDSP_maxviD(d, 1, &r, &i, 6);
	expectNear(r, 3.6, 1e-12, "maxviD", 0);
	expectIndex(i, 2, "maxviD");
	vDSP_minviD(d, 1, &r, &i, 6);
	expectNear(r, -4.3, 1e-12, "minviD", 0);
	expectIndex(i, 5, "minviD");

	// N equal to zero: -infinity or infinity and an index of 0, and A is not read.
	c = 0;
	i = 99;
	vDSP_maxvi(NULL, 1, &c, &i, 0);
	if (!isinf(c) || c > 0)
	{
		printf("FAIL maxvi empty: got %g\n", c);
		g_failures++;
	}
	expectIndex(i, 0, "maxvi empty");
	c = 0;
	i = 99;
	vDSP_minvi(NULL, 1, &c, &i, 0);
	if (!isinf(c) || c < 0)
	{
		printf("FAIL minvi empty: got %g\n", c);
		g_failures++;
	}
	expectIndex(i, 0, "minvi empty");
	r = 0;
	i = 99;
	vDSP_maxviD(NULL, 1, &r, &i, 0);
	if (!isinf(r) || r > 0)
	{
		printf("FAIL maxviD empty: got %g\n", r);
		g_failures++;
	}
	expectIndex(i, 0, "maxviD empty");
	r = 0;
	i = 99;
	vDSP_minviD(NULL, 1, &r, &i, 0);
	if (!isinf(r) || r < 0)
	{
		printf("FAIL minviD empty: got %g\n", r);
		g_failures++;
	}
	expectIndex(i, 0, "minviD empty");

	// Stride: every other element is -1.5, 3.6, -0.1, so the maximum 3.6 is element 1 of the strided vector, at
	// offset 1 * 2 of A. The public vDSP.h (macOS 10.15 SDK) gives I as the smallest offset i*IA of an element equal
	// to C, so I counts elements of A, not of the strided vector.
	vDSP_maxvi(a, 2, &c, &i, 3);
	expectNear(c, 3.6, 1e-6, "maxvi stride", 0);
	expectIndex(i, 2, "maxvi stride");
	// Every third element is -1.5, 0.2: the minimum -1.5 is at offset 0, and (double precision) the maximum 0.2 is
	// at offset 3.
	vDSP_minvi(a, 3, &c, &i, 2);
	expectNear(c, -1.5, 1e-6, "minvi stride", 0);
	expectIndex(i, 0, "minvi stride");
	vDSP_maxviD(d, 3, &r, &i, 2);
	expectNear(r, 0.2, 1e-12, "maxviD stride", 0);
	expectIndex(i, 3, "maxviD stride");
	// GUESS (negative stride): it walks backwards from the pointer given, and I is the wrapped (vDSP_Length) (n * IA)
	// since an unsigned index cannot hold the negative offset; the header does not cover it. Here -4.3, -0.1, 0.2 are
	// read first (offset 0), and 3.6 is element 3 of six read from a + 5 (offset -3, wrapped).
	vDSP_minviD(d + 5, -1, &r, &i, 3);
	expectNear(r, -4.3, 1e-12, "minviD negative stride", 0);
	expectIndex(i, 0, "minviD negative stride");
	vDSP_maxvi(a + 5, -1, &c, &i, 6);
	expectNear(c, 3.6, 1e-6, "maxvi negative stride", 0);
	expectIndex(i, (vDSP_Length) -3L, "maxvi negative stride");

	// Ties: the first occurrence is reported (the smallest offset i*IA, per the public vDSP.h).
	const float ties[] = { 1, 5, 2, 5, 1, 0, 0 };
	vDSP_maxvi(ties, 1, &c, &i, 7);
	expectNear(c, 5, 0, "maxvi tie", 0);
	expectIndex(i, 1, "maxvi tie");
	vDSP_minvi(ties, 1, &c, &i, 7);
	expectNear(c, 0, 0, "minvi tie", 0);
	expectIndex(i, 5, "minvi tie");
	const double tiesD[] = { 1, 5, 2, 5, 1, 0, 0 };
	vDSP_maxviD(tiesD, 1, &r, &i, 7);
	expectIndex(i, 1, "maxviD tie");
	vDSP_minviD(tiesD, 1, &r, &i, 7);
	expectIndex(i, 5, "minviD tie");

	// GUESS (NaN): a NaN is skipped, wherever it sits; only NaNs gives -infinity or infinity and an index of 0.
	const float withNaN[] = { NAN, 1, NAN, 4, 2 };
	vDSP_maxvi(withNaN, 1, &c, &i, 5);
	expectNear(c, 4, 0, "maxvi nan", 0);
	expectIndex(i, 3, "maxvi nan");
	vDSP_minvi(withNaN, 1, &c, &i, 5);
	expectNear(c, 1, 0, "minvi nan", 0);
	expectIndex(i, 1, "minvi nan");
	const float onlyNaN[] = { NAN, NAN };
	i = 99;
	vDSP_maxvi(onlyNaN, 1, &c, &i, 2);
	if (!isinf(c) || c > 0)
	{
		printf("FAIL maxvi only NaN: got %g\n", c);
		g_failures++;
	}
	expectIndex(i, 0, "maxvi only NaN");
}

int main(void)
{
	testArithmetic();
	testRealVectorOps();
	testReductions();
	testExtremaIndex();
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
