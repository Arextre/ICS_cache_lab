/*
 *csim.c-使用C编写一个Cache模拟器，它可以处理来自Valgrind的跟踪和输出统计
 *息，如命中、未命中和逐出的次数。更换政策是LRU。
 * 设计和假设:
 *  1. 每个加载/存储最多可导致一个缓存未命中。（最大请求是8个字节。）
 *  2. 忽略指令负载（I），因为我们有兴趣评估trace.c内容中数据存储性能。
 *  3. 数据修改（M）被视为加载，然后存储到同一地址。因此，M操作可能导致两次缓存命中，或者一次未命中和一次命中，外加一次可能的逐出。
 * 使用函数printSummary() 打印输出，输出hits, misses and evictions 的数，这对结果评估很重要
*/
#include "cachelab.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>

//                    请在此处添加代码  
//****************************Begin*********************
int s, E, b, verbose;
char *trace_file = NULL;

void parse_args(int argc, char **argv) {
  int opt;
  extern char *optarg;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
      printf("Options:\n");
      printf("  -h         Print this help message.\n");
      printf("  -v         Optional verbose flag.\n");
      printf("  -s <num>   Number of set index bits (S = 2^s is the number of sets).\n");
      printf("  -E <num>   Number of lines per set (associativity).\n");
      printf("  -b <num>   Number of block offset bits (B = 2^b is the block size in bytes).\n");
      printf("  -t <file>  Trace file.\n");
      exit(0);
    case 'v':
      verbose = 1;
      break;
    case 's':
      s = atoi(optarg);
      break;
    case 'E':
      E = atoi(optarg);
      break;
    case 'b':
      b = atoi(optarg);
      break;
    case 't':
      trace_file = optarg;
      break;
    default:
      fprintf(stderr, "Invalid option: -%c\n", opt);
      exit(1);
    }
  }
}


/* Cache definition and operations */
typedef unsigned long address_t;
typedef struct {
  int valid;
  address_t tag;
  unsigned long lru_counter;  // used for LRU eviction policy
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;
address_t S;
address_t B;
address_t set_index_mask;
cache_t cache;
unsigned long global_lru_counter = 0;

void initCache() {
  S = 1 << s;
  B = 1 << b;
  cache = (cache_t)malloc(S * sizeof(cache_set_t));
  set_index_mask = (S - 1) << b;
  for (unsigned long i = 0; i < S; ++i) {
    cache[i] = (cache_set_t)malloc(E * sizeof(cache_line_t));
    for (int j = 0; j < E; ++j) {
      cache[i][j].valid = 0;
      cache[i][j].tag = 0;
      cache[i][j].lru_counter = 0;
    }
  }
}

void freeCache() {
  for (unsigned long i = 0; i < S; ++i)
    free(cache[i]);
  free(cache);
}

void accessCache(address_t addr, int *hits, int *misses, int *evictions) {
  address_t set_index = (addr & set_index_mask) >> b;
  address_t tag = addr >> (s + b);
  // pointer to the cache set
  cache_set_t set = cache[set_index];
  // Check for hit
  unsigned long lru_min = (unsigned long)-1; // init to max value of unsigned long
  int lru_index = -1;
  for (int i = 0; i < E; ++i) {
    if (set[i].valid && set[i].tag == tag) {
      ++(*hits);
      set[i].lru_counter = ++global_lru_counter;  // update LRU counter
      return ;
    } else if (!set[i].valid) {
      // empty line
      lru_min = 0;    // empty lines are considered least recently used
      lru_index = i;
    } else if (lru_index == -1 || set[i].lru_counter < lru_min) {
      lru_min = set[i].lru_counter;
      lru_index = i;
    }
  }
  // Miss occurred
  ++(*misses);
  if (set[lru_index].valid) {
    // Eviction needed
    ++(*evictions);
  }
  set[lru_index].valid = 1;
  set[lru_index].tag = tag;
  set[lru_index].lru_counter = ++global_lru_counter;  // update LRU counter
}

void accessData(
  address_t addr, address_t size,
  int *hits, int *misses, int *evictions
) {
  address_t start = (addr >> b) << b;
  address_t end = ((addr + size - 1) >> b) << b;
  // the task presumes that accesses are aligned and do not span multiple blocks
  assert (start == end);
  for (address_t cur = start; cur <= end; cur += B) {
    accessCache(cur, hits, misses, evictions);
  }
}

int main(int argc, char **argv) {
  // initialize variables
  parse_args(argc, argv);
  FILE *fp = fopen(trace_file, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error opening trace file: %s\n", trace_file);
    exit(1);
  }
  if (verbose) {
    printf("Verbose Mode: ON\n");
    printf("Set index bits (s): %d\n", s);
    printf("Lines per set (E): %d\n", E);
    printf("Block offset bits (b): %d\n", b);
    printf("Trace file: %s\n", trace_file);
  }
  initCache();

  // read and process trace file
  int hits = 0, misses = 0, evictions = 0;
  char buf[1024];
  char op;
  unsigned long addr, size;
  
  while (fgets(buf, sizeof(buf), fp) != NULL) {

    if (buf[0] == 'I') {
      // I operation is ignored
      if (verbose) {
        printf("Skipping instruction load: %s", buf);
      }
      continue;
    }

    sscanf(buf, " %c %lx,%lu", &op, &addr, &size);
    if (verbose) {
      printf("Operation: %c, Address: 0x%lx, Size: %lu\n", op, addr, size);
    }
    accessData(addr, size, &hits, &misses, &evictions);
    if (op == 'M') {
      // M operation causes an additional access
      accessData(addr, size, &hits, &misses, &evictions);
    }
    if (verbose) {
      printf("Current hits: %d, misses: %d, evictions: %d\n", hits, misses, evictions);
    }
  }

  freeCache();
  printSummary(hits, misses, evictions); 
  return 0;
}
//****************************End**********************#