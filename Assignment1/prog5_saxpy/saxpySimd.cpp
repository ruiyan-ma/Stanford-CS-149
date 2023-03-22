#include <immintrin.h>

void saxpySimd(int N,
                       float scale,
                       float X[],
                       float Y[],
                       float result[])
{
    int i = 0;
    __m256 scaleVec = _mm256_set1_ps(scale);
    for (; i < N; i += 8) {
        __m256 output = _mm256_fmadd_ps(scaleVec, _mm256_loadu_ps(X + i), _mm256_loadu_ps(Y + i));
        _mm256_stream_ps(result + i, output);
    }

    if (i == N) return;
    i -= 8;
    for (; i < N; i++) {
        result[i] = scale * X[i] + Y[i];
    }
}

