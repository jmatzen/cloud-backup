#include "Base58.h"

namespace
{
  const char ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

  typedef uint8_t byte;

  byte divmod58(byte* number, size_t n, int start) {
    int r = 0;
    for (int i = start; i < n; ++i) {
      int tmp = r * 256 + number[i];
      number[i] = tmp / 58;
      r = tmp % 58;
    }
    return r;
  }
}

std::string jm::base58encode(const byte* const input_, const size_t n)
{
  if (!input_ || !n)
    return std::string();
  byte* const input = (byte*)alloca(n);
  memcpy(input, input_, n);

  int zc = 0;
  while (zc < n && input[zc] == 0) ++zc;
  const size_t tmplen = n * 2;
  char* const tmp = (char*)alloca(tmplen);
  size_t j = tmplen;
  int s = zc;
  while (s < n) {
    uint8_t m = divmod58(input, n, s);
    if (input[s] == 0) {
      ++s;
    }
    tmp[--j] = ALPHABET[m];
  }
  while (j < tmplen && tmp[j] == ALPHABET[0]) ++j;
  while (--zc >= 0) {
    tmp[--j] = ALPHABET[0];
  }
  return std::string(tmp + j, tmp + tmplen);
}

