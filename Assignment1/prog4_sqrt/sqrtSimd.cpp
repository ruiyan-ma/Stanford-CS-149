#include <immintrin.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

void sqrtSimd(int N,
                float initialGuess,
                float values[],
                float output[])
{
    static const float kThreshold = 0.00001f;

    __m256 kThresholdsimd = _mm256_set1_ps(kThreshold);

    __m256 cmp1f = _mm256_set1_ps(1.f);
    __m256 cmp05f = _mm256_set1_ps(0.5f);
    __m256 cmp3f = _mm256_set1_ps(3.f);

    __m256 fabsimd = _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff));

    __m256 x, guess, guess3, errorbeforeabs, error, result, cmpres;

    int i = 0;
    for (; i < N; i += 8) {
        x = _mm256_loadu_ps(values + i);
        guess = _mm256_set1_ps(initialGuess);
        errorbeforeabs = _mm256_fmsub_ps(_mm256_mul_ps(guess, guess), x, cmp1f);

        error = _mm256_and_ps(errorbeforeabs, fabsimd);
        cmpres = _mm256_cmp_ps(error, kThresholdsimd, _CMP_GT_OQ);

        while (_mm256_movemask_ps(cmpres)) {
            guess3 = _mm256_mul_ps(guess, _mm256_mul_ps(guess, guess));
            __m256 newguess = _mm256_mul_ps(_mm256_sub_ps(_mm256_mul_ps(cmp3f, guess), _mm256_mul_ps(x, guess3)), cmp05f);
            guess = _mm256_blendv_ps(guess, newguess, cmpres);
            error = _mm256_and_ps(_mm256_sub_ps(_mm256_mul_ps(_mm256_mul_ps(guess, guess), x), cmp1f), fabsimd);
            cmpres = _mm256_cmp_ps(error, kThresholdsimd, _CMP_GT_OQ);
        }

        result = _mm256_mul_ps(guess, x);
        _mm256_storeu_ps(output + i, result);
    }    

    if (i == N) return;

    i -= 8;

    for (; i < N; i++) {

        float x = values[i];
        float guess = initialGuess;

        float error = fabs(guess * guess * x - 1.f);

        while (error > kThreshold) {
            guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
            error = fabs(guess * guess * x - 1.f);
        }

        output[i] = x * guess;
    } 
}