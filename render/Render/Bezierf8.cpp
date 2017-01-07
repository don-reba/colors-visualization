// Borrowed from WebKit.

#include "Bezierf8.h"
#include <math.h>

using namespace Eigen;

Bezierf8::Bezierf8(Vector2f p1, Vector2f p2)
{
	// calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1)
	float cxf = 3.0f * p1.x();
	float bxf = 3.0f * (p2.x() - p1.x()) - cxf;
	float axf = 1.0f - cxf - bxf;

	float cyf = 3.0f * p1.y();
	float byf = 3.0f * (p2.y() - p1.y()) - cyf;
	float ayf = 1.0f - cyf - byf;

	cx = _mm256_set1_ps(cxf);
	bx = _mm256_set1_ps(bxf);
	ax = _mm256_set1_ps(axf);

	cy = _mm256_set1_ps(cyf);
	by = _mm256_set1_ps(byf);
	ay = _mm256_set1_ps(ayf);
}

__m256 Bezierf8::SampleCurveX(__m256 t) const
{
	// ax·t³ + bx·t² + cx·t = ((ax·t + bx)t + cx)t
	return _mm256_mul_ps(_mm256_fmadd_ps(_mm256_fmadd_ps(ax, t, bx), t, cx), t);
}

__m256 Bezierf8::SampleCurveY(__m256 t) const
{
	// ay·t³ + by·t² + cy·t = ((ay·t + by)t + cy)t
	return _mm256_mul_ps(_mm256_fmadd_ps(_mm256_fmadd_ps(ay, t, by), t, cy), t);
}

__m256 Bezierf8::SampleCurveDerivativeX(__m256 t) const
{
	// 3·ax·t² + 2·bx·t + cx = (3·ax·t + 2·bx)t + cx
	__m256 ax3 = _mm256_add_ps(_mm256_add_ps(ax, ax), ax);
	__m256 bx2 = _mm256_add_ps(bx, bx);
	return _mm256_fmadd_ps(_mm256_fmadd_ps(ax3, t, bx2), t, cx);
}

// Given an x value, find a parametric value it came from.
__m256 Bezierf8::SolveCurveX(__m256 x, __m256 epsilon) const
{
	__m256 signMask = _mm256_set1_ps(-0.0f);

	// try a few Newton's method iterations
	__m256 t = x;
	for (size_t i = 0; i < 8; i++) {
		__m256 x2 = _mm256_sub_ps(SampleCurveX(t), x);
		__m256 d2 = SampleCurveDerivativeX(t);

		__m256 fabs_x2 = _mm256_andnot_ps(signMask, x2);
		
		t = _mm256_blendv_ps
			( _mm256_sub_ps(t, _mm256_div_ps(x2, d2))
			, t
			, _mm256_cmp_ps(fabs_x2, epsilon, _CMP_LT_OS)
			);
	}

	return t;
}

__m256 Bezierf8::Solve(__m256 x, __m256 epsilon) const
{
	return SampleCurveY(SolveCurveX(x, epsilon));
}