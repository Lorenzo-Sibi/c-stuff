#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>


#define HLL_HASH_SIZE 64
#define HLL_P 16      // Number of bits used for the index (should be 4 <= p <= 16)
#define HLL_R (HLL_HASH_SIZE - HLL_P)
#define HLL_M (1 << HLL_P)
#define HLL_INDEX_MASK ( HLL_M - 1)

#define HLL_ALPHA (0.7213 / (1 + 1.079 / HLL_M))

#define HLL_E_FILTER_1 (2.5f * HLL_M)
#define HLL_E_FILTER_2 ((1ULL << 32) / 30.0f)


typedef struct {
    uint8_t registers[HLL_M];   // The registers have to count the leading zeroes, since the hash is 64 bits long, the registers are 8 bits long.
    uint32_t zero_registers;    // The number of registers equal to 0.
} HyperLogLog;

uint64_t MurmurHash64A ( const void * key, int len, uint64_t seed )
{
  const uint64_t m = 0xc6a4a7935bd1e995;
  const int r = 47;

  uint64_t h = seed ^ (len * m);

  const uint64_t * data = (const uint64_t *)key;
  const uint64_t * end = data + (len/8);

  while(data != end)
  {
    uint64_t k = *data++;

    k *= m; 
    k ^= k >> r; 
    k *= m; 
    
    h ^= k;
    h *= m; 
  }

  const unsigned char * data2 = (const unsigned char*)data;

  switch(len & 7)
  {
  case 7: h ^= ((uint64_t) data2[6]) << 48; /* fall through */
  case 6: h ^= ((uint64_t) data2[5]) << 40; /* fall through */
  case 5: h ^= ((uint64_t) data2[4]) << 32; /* fall through */
  case 4: h ^= ((uint64_t) data2[3]) << 24; /* fall through */
  case 3: h ^= ((uint64_t) data2[2]) << 16; /* fall through */
  case 2: h ^= ((uint64_t) data2[1]) << 8;  /* fall through */
  case 1: h ^= ((uint64_t) data2[0]);       /* fall through */
          h *= m;
  };
 
  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
} 


void hllInit(HyperLogLog* hll) {
    memset(hll->registers, 0, sizeof(hll->registers));
    hll->zero_registers = HLL_M;
}

void hllAggregate(HyperLogLog* hll, void* data, size_t size) {
    uint64_t hash, index;
    uint8_t count;
    
    hash = MurmurHash64A(data, size, 0);
    index = hash & HLL_INDEX_MASK;

    hash = hash >> HLL_P; // Discarding the first p bits used for the index
    count = __builtin_clzll(hash) + 1; // Count leading zeros

    if (count > hll->registers[index]) {
        if (hll->registers[index] == 0)
            hll->zero_registers--;
        hll->registers[index] = count;
    }
}

double hllCount(HyperLogLog* hll) {    
    double sum, E = 0;
    uint8_t* registers = hll->registers;

    for (int i = 0; i < HLL_M; i++)
        sum += pow(2.0, -registers[i]);
    sum = 1 / sum;

    E = HLL_ALPHA * (HLL_M << 1) * sum;

    if (E <= HLL_E_FILTER_1) {
        uint32_t V = hll->zero_registers;
        if (V != 0)
            return HLL_M * log(HLL_M / (double)V);
        else
            return E;
    } else if (E <= HLL_E_FILTER_2)
        return E;
    else
        return - (1ULL << 32) * log(1 - (E / (1ULL << 32)));
}

void stampa_binario64(uint64_t n) {
    for (int i = 63; i >= 0; i--) {
        printf("%llu", (n >> i) & 1ULL);
    }
    printf("\n");
}


int main() {

    HyperLogLog hll;
    hllInit(&hll);

    char* data[] = {"hello", "world", "hello", "hyperloglog", "world"};
    size_t data_size = sizeof(data) / sizeof(data[0]);
    
    for (size_t i = 0; i < data_size; i++)
        hllAggregate(&hll, data[i], strlen(data[i]));

    double count = hllCount(&hll);
    printf("Estimated unique count: %d\n", (int) count);

    return 0;
}