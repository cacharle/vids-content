#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>

#define SIZE (1UL << 33)
#define WRITES 1000000000UL

int main(void) {
	char *mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
	                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_POPULATE, -1, 0);
	if (mem == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	uint64_t s = 42;

	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);

	for (unsigned long i = 0; i < WRITES; i++) {
		s ^= s << 13; s ^= s >> 7; s ^= s << 17;
		unsigned long off = s & (SIZE - 1);
		mem[off] = (char)i;
	}

	clock_gettime(CLOCK_MONOTONIC, &t1);

	double elapsed = (t1.tv_sec - t0.tv_sec) +
	                 (t1.tv_nsec - t0.tv_nsec) / 1e9;
	printf("%lu random writes in %.3f s (%.0f ns/write)\n",
	       WRITES, elapsed, elapsed * 1e9 / WRITES);

	munmap(mem, SIZE);
	return 0;
}
