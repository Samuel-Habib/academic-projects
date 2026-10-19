#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <vector>
#include <webp/encode.h>

using namespace std;

static void write_ppm(const char *path, const uint32_t *rgba, int w, int h) {
  std::ofstream f(path, std::ios::binary);
  if (!f)
    std::exit(1);
  f << "P6\n" << w << " " << h << "\n255\n";
  for (long i = 0; i < 1L * w * h; i++) {
    uint8_t r = (rgba[i] >> 16) & 0xFF;
    uint8_t g = (rgba[i] >> 8) & 0xFF;
    uint8_t b = (rgba[i]) & 0xFF;
    f.write(reinterpret_cast<char *>(&r), 1);
    f.write(reinterpret_cast<char *>(&g), 1);
    f.write(reinterpret_cast<char *>(&b), 1);
  }
}

static bool save_webp(const char *filename, const uint32_t *pixels, int w,
                      int h, int quality) {
  uint8_t *data = nullptr;
  size_t sz = WebPEncodeRGBA(reinterpret_cast<const uint8_t *>(pixels), w, h,
                             w * 4, quality, &data);
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

static void counts_to_rgba(uint32_t *out, const uint32_t *cnt, int w, int h,
                           const uint32_t pal[256]) {
  const long N = 1L * w * h;
  for (long i = 0; i < N; i++)
    out[i] = pal[cnt[i] & 255];
}

static void mandelbrot_avx2(uint32_t *counts, int w, int h, int max_iter,
                            float xmin, float xmax, float ymin, float ymax) {
  const int V = 8;
  const float dx = (xmax - xmin) / float(w);
  const float dy = (ymax - ymin) / float(h);

  const __m256 four = _mm256_set1_ps(4.0f);
  const __m256 fdx = _mm256_set1_ps(dx);
  const __m256 lane = _mm256_setr_ps(0, 1, 2, 3, 4, 5, 6, 7);

  for (int y = 0; y < h; y++) {
    const float cy = ymin + dy * float(y);
    const __m256 c_im = _mm256_set1_ps(cy);

    int x = 0;
    for (; x + V <= w; x += V) {
      const float x0 = xmin + dx * float(x);
      const __m256 c_re = _mm256_fmadd_ps(lane, fdx, _mm256_set1_ps(x0));

      __m256 zr = _mm256_setzero_ps();
      __m256 zi = _mm256_setzero_ps();
      __m256i it = _mm256_setzero_si256();
      __m256 active = _mm256_castsi256_ps(_mm256_set1_epi32(-1));

      for (int k = 0; k < max_iter; k++) {
        __m256 zr2 = _mm256_mul_ps(zr, zr);
        __m256 zi2 = _mm256_mul_ps(zi, zi);
        __m256 mag2 = _mm256_add_ps(zr2, zi2);

        __m256 inside = _mm256_cmp_ps(mag2, four, _CMP_LT_OQ);
        __m256 new_active = _mm256_and_ps(active, inside);
        if (_mm256_movemask_ps(new_active) == 0)
          break;

        __m256i ones = _mm256_set1_epi32(1);
        __m256i mask_i = _mm256_castps_si256(new_active);
        it = _mm256_add_epi32(it, _mm256_and_si256(mask_i, ones));

        __m256 zrzi = _mm256_mul_ps(zr, zi);
        __m256 zi_next = _mm256_add_ps(_mm256_add_ps(zrzi, zrzi), c_im);
        __m256 zr_next = _mm256_add_ps(_mm256_sub_ps(zr2, zi2), c_re);

        zr = _mm256_blendv_ps(zr, zr_next, new_active);
        zi = _mm256_blendv_ps(zi, zi_next, new_active);
        active = new_active;
      }

      _mm256_storeu_si256(reinterpret_cast<__m256i *>(&counts[1L * y * w + x]),
                          it);
    }

    for (; x < w; x++) {
      float cr = xmin + dx * float(x);
      float zr = 0.0f, zi = 0.0f;
      int k = 0;
      while (k < max_iter && (zr * zr + zi * zi) < 4.0f) {
        float zr2 = zr * zr, zi2 = zi * zi;
        float nzr = zr2 - zi2 + cr;
        float nzi = 2.0f * zr * zi + cy;
        zr = nzr;
        zi = nzi;
        k++;
      }
      counts[1L * y * w + x] = uint32_t(k);
    }
  }
}

int main() {
  const int W = 1920, H = 1080;
  const int MAX_IT = 512;
  const float XMIN = -2.0f, XMAX = 0.47f;
  const float YMIN = -1.12f, YMAX = 1.12f;

  std::vector<uint32_t> counts(1L * W * H);
  std::vector<uint32_t> pixels(1L * W * H);
  uint32_t palette[256];
  make_palette(palette);

  auto t0 = std::chrono::high_resolution_clock::now();
  mandelbrot_avx2(counts.data(), W, H, MAX_IT, XMIN, XMAX, YMIN, YMAX);
  auto t1 = std::chrono::high_resolution_clock::now();

  counts_to_rgba(pixels.data(), counts.data(), W, H, palette);
  write_ppm("mandelbrot.ppm", pixels.data(), W, H);

  bool ok = save_webp("mandelbrot.webp", pixels.data(), W, H, 90);
  if (!ok)
    std::cerr << "webp encode failed\n";

  std::chrono::duration<double> dt = t1 - t0;
  std::cout << dt.count() << "\n";
  return 0;
}
