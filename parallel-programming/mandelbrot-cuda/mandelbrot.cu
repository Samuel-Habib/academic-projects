// Samuel Habib

/*
GPU: NVIDIA A100-SXM4-40GB
Compute capability: 8.0
Global memory (GiB): 39.5574
CPU scalar seconds: 0.597128
GPU kernel-only seconds: 0.000181248
GPU total: 0.00154744
Speedup: 385.882
*/

// nvcc -O3 -std=c++17 -arch=sm_80 --fmad=false -Xcompiler -ffp-contract=off mandelbrot.cu -o mandelbrot -lwebp

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <cuda_runtime.h>

#define USE_WEBP 1
#include <webp/encode.h>

using namespace std;

#define CUDA_CHECK(x)                                                         \
  do {                                                                        \
    cudaError_t err = (x);                                                    \
    if (err != cudaSuccess) {                                                 \
      std::cerr << "CUDA error: " << cudaGetErrorString(err)                  \
                << " (" << __FILE__ << ":" << __LINE__ << ")\n";              \
      std::exit(1);                                                           \
    }                                                                         \
  } while (0)

static bool save_webp(const char *filename, const uint32_t *pixels, int w, int h, int quality) {
  uint8_t *data = nullptr;
  size_t sz = WebPEncodeBGRA(reinterpret_cast<const uint8_t *>(pixels), w, h, w * 4, quality, &data);
  if (sz == 0 || !data)
    return false;
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    WebPFree(data);
    return false;
  }
  file.write(reinterpret_cast<const char *>(data), sz);
  file.close();
  WebPFree(data);
  return true;
}

static void make_palette(uint32_t tab[256]) {
  for (int i = 0; i < 256; i++) {
    uint8_t r = (i * 5) & 255;
    uint8_t g = (i * 7) & 255;
    uint8_t b = (i * 11) & 255;
    tab[i] = 0xFF000000u | (uint32_t(r) << 16) | (uint32_t(g) << 8) | b;
  }
}

static void counts_to_rgba(uint32_t *out, const uint32_t *cnt, int w, int h, const uint32_t pal[256], uint32_t maxcount) {
  const long N = 1L * w * h;
  for (long i = 0; i < N; i++) {
    if (cnt[i] >= maxcount)
      out[i] = 0xFF000000u;
    else
      out[i] = pal[cnt[i] & 255u];
  }
}

static void mandelbrot_scalar(uint32_t *counts, uint32_t w, uint32_t h, uint32_t max_iter, float xmin, float xmax, float ymin, float ymax) {
  const float dx = (xmax - xmin) / float(w);
  const float dy = (ymax - ymin) / float(h);

  for (uint32_t y = 0; y < h; y++) {
    const float cy = ymin + dy * float(y);
    for (uint32_t x = 0; x < w; x++) {
      const float cr = xmin + dx * float(x);
      float zr = 0.0f, zi = 0.0f;
      uint32_t k = 0;
      while (k < max_iter && (zr * zr + zi * zi) < 4.0f) {
        float zr2 = zr * zr, zi2 = zi * zi;
        float nzr = zr2 - zi2 + cr;
        float nzi = 2.0f * zr * zi + cy;
        zr = nzr;
        zi = nzi;
        k++;
      }
      counts[1ULL * y * w + x] = k;
    }
  }
}

__global__ static void mandelbrot_kernel(uint32_t *counts, uint32_t w, uint32_t h, uint32_t max_iter, float xmin, float ymin, float dx, float dy) {
  const uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
  const uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;
  if (x >= w || y >= h)
    return;

  const float cr = xmin + dx * float(x);
  const float ci = ymin + dy * float(y);

  float zr = 0.0f;
  float zi = 0.0f;
  uint32_t k = 0;

  while (k < max_iter) {
    const float zr2 = zr * zr;
    const float zi2 = zi * zi;
    if (zr2 + zi2 >= 4.0f)
      break;

    
    float nzr = zr2 - zi2 + cr;
    float nzi = 2.0f * zr * zi + ci;
    zr = nzr;
    zi = nzi;
    k++;
  }

  counts[1ULL * y * w + x] = k;
}

extern "C" void mandelbrot(uint32_t counts[], uint32_t maxcount, uint32_t w, uint32_t h, float xmin, float xmax, float ymin, float ymax) {
  static uint32_t *d_counts = nullptr;
  static size_t d_bytes = 0;

  const size_t bytes = 1ULL * w * h * sizeof(uint32_t);
  if (bytes != d_bytes) {
    if (d_counts)
      CUDA_CHECK(cudaFree(d_counts));
    CUDA_CHECK(cudaMalloc(&d_counts, bytes));
    d_bytes = bytes;
  }

  dim3 block(16, 16);
  dim3 grid((w + block.x - 1) / block.x, (h + block.y - 1) / block.y);

  const float dx = (xmax - xmin) / float(w);
  const float dy = (ymax - ymin) / float(h);

  mandelbrot_kernel<<<grid, block>>>(d_counts, w, h, maxcount, xmin, ymin, dx, dy);
  CUDA_CHECK(cudaGetLastError());
  CUDA_CHECK(cudaDeviceSynchronize());

  CUDA_CHECK(cudaMemcpy(counts, d_counts, bytes, cudaMemcpyDeviceToHost));
}

static void print_gpu_info() {
  int dev = 0;
  CUDA_CHECK(cudaGetDevice(&dev));
  cudaDeviceProp prop;
  CUDA_CHECK(cudaGetDeviceProperties(&prop, dev));
  std::cout << "GPU: " << prop.name << "\n";
  std::cout << "Compute capability: " << prop.major << "." << prop.minor << "\n";
  std::cout << "Global memory (GiB): " << double(prop.totalGlobalMem) / (1024.0 * 1024.0 * 1024.0) << "\n";
}

static double gpu_kernel_time_seconds(uint32_t *d_counts, uint32_t w, uint32_t h, uint32_t maxcount, float xmin, float xmax, float ymin, float ymax) {
  dim3 block(16, 16);
  dim3 grid((w + block.x - 1) / block.x, (h + block.y - 1) / block.y);

  const float dx = (xmax - xmin) / float(w);
  const float dy = (ymax - ymin) / float(h);

  cudaEvent_t start, stop;
  CUDA_CHECK(cudaEventCreate(&start));
  CUDA_CHECK(cudaEventCreate(&stop));

  mandelbrot_kernel<<<grid, block>>>(d_counts, w, h, maxcount, xmin, ymin, dx, dy);
  CUDA_CHECK(cudaGetLastError());

  CUDA_CHECK(cudaEventRecord(start));
  mandelbrot_kernel<<<grid, block>>>(d_counts, w, h, maxcount, xmin, ymin, dx, dy);
  CUDA_CHECK(cudaGetLastError());
  CUDA_CHECK(cudaEventRecord(stop));
  CUDA_CHECK(cudaEventSynchronize(stop));

  float ms = 0.0f;
  CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));

  CUDA_CHECK(cudaEventDestroy(start));
  CUDA_CHECK(cudaEventDestroy(stop));

  return double(ms) / 1000.0;
}

int main(int argc, char **argv) {
  const uint32_t W = 1280, H = 1024;
  const uint32_t MAX_IT = 256;
  const float XMIN = -1.0f, XMAX = 1.0f;
  const float YMIN = -1.0f, YMAX = 1.0f;

  (void)argc;
  (void)argv;

  print_gpu_info();

  std::vector<uint32_t> counts_cpu(1ULL * W * H);
  std::vector<uint32_t> counts_gpu(1ULL * W * H);
  std::vector<uint32_t> pixels(1ULL * W * H);

  uint32_t palette[256];
  make_palette(palette);

  auto t0 = std::chrono::high_resolution_clock::now();
  mandelbrot_scalar(counts_cpu.data(), W, H, MAX_IT, XMIN, XMAX, YMIN, YMAX);
  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> dt_cpu = t1 - t0;

  uint32_t *d_counts = nullptr;
  const size_t bytes = 1ULL * W * H * sizeof(uint32_t);
  CUDA_CHECK(cudaMalloc(&d_counts, bytes));

  const double dt_gpu_kernel = gpu_kernel_time_seconds(d_counts, W, H, MAX_IT, XMIN, XMAX, YMIN, YMAX);

  auto g0 = std::chrono::high_resolution_clock::now();
  mandelbrot(counts_gpu.data(), MAX_IT, W, H, XMIN, XMAX, YMIN, YMAX);
  auto g1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> dt_gpu_total = g1 - g0;

  CUDA_CHECK(cudaFree(d_counts));
  
  counts_to_rgba(pixels.data(), counts_gpu.data(), int(W), int(H), palette, MAX_IT);

  bool ok = save_webp("mandelbrot.webp", pixels.data(), int(W), int(H), 90);
  if (!ok)
    std::cerr << "webp encode failed\n";

  const double cpu_s = dt_cpu.count();
  const double gpu_total_s = dt_gpu_total.count();

  std::cout << "CPU scalar seconds: " << cpu_s << "\n";
  std::cout << "GPU kernel-only seconds: " << dt_gpu_kernel << "\n";
  std::cout << "GPU total: " << gpu_total_s << "\n";
  if (gpu_total_s > 0.0)
    std::cout << "Speedup: " << (cpu_s / gpu_total_s) << "\n";

  return 0;
}
