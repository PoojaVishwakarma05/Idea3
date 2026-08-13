#include <time.h>
#include <omp.h>
 
static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}
 
int main(int argc, char **argv) {
    long long n = 1024;
    const char *outfile = NULL;
 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            n = atoll(argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        }
    }
    if (n <= 0) {
        fprintf(stderr, "invalid matrix size: %lld\n", n);
        return 1;
    }
 
    int threads = omp_get_max_threads();
    fprintf(stderr, "matmul: N=%lld threads=%d\n", n, threads);
 
    double *A = malloc((size_t)n * n * sizeof(double));
    double *B = malloc((size_t)n * n * sizeof(double));
    double *C = calloc((size_t)n * n, sizeof(double));
    if (!A || !B || !C) {
        perror("malloc");
        return 1;
    }
 
    srand(42);
    for (long long i = 0; i < n * n; i++) {
        A[i] = (double)(rand() % 100) / 10.0;
        B[i] = (double)(rand() % 100) / 10.0;
    }
 
    double t0 = now_ms();
#pragma omp parallel for
    for (long long i = 0; i < n; i++) {
        for (long long k = 0; k < n; k++) {
            double aik = A[i * n + k];
            for (long long j = 0; j < n; j++) {
                C[i * n + j] += aik * B[k * n + j];
            }
        }
    }
    double dt = now_ms() - t0;
 
    double flops = 2.0 * (double)n * n * n;
    double gflops = flops / (dt / 1000.0) / 1e9;
 
    double sum = 0;
    for (long long i = 0; i < n * n; i++) sum += C[i];
 
    printf("N=%lld threads=%d time=%.3f ms GFLOPS=%.2f checksum=%.6f\n",
           n, threads, dt, gflops, sum);
 
    if (outfile) {
        FILE *f = fopen(outfile, "w");
        if (!f) {
            perror("fopen");
            return 1;
        }
        fprintf(f, "N=%lld threads=%d time=%.3f ms GFLOPS=%.2f checksum=%.6f\n",
                n, threads, dt, gflops, sum);
        fclose(f);
    }
 
    free(A);
    free(B);
    free(C);
    return 0;
}
