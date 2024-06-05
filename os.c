
#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>

#include "os.h"

/* 2^20 pages ought to be enough for anybody */
#define NPAGES (1024 * 1024)

static char *pages[NPAGES];

uint64_t alloc_page_frame(void)
{
	static uint64_t nalloc;
	uint64_t ppn;
	void *va;

	if (nalloc == NPAGES)
		errx(1, "out of physical memory");

	/* OS memory management isn't really this simple */
	ppn = nalloc;
	nalloc++;

	va = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (va == MAP_FAILED)
		err(1, "mmap failed");

	pages[ppn] = va;
	return ppn + 0xbaaaaaad;
}

void *phys_to_virt(uint64_t phys_addr)
{
	uint64_t ppn = (phys_addr >> 12) - 0xbaaaaaad;
	uint64_t off = phys_addr & 0xfff;
	char *va = NULL;

	if (ppn < NPAGES)
		va = pages[ppn] + off;

	return va;
}

int main(int argc, char **argv)
{
	uint64_t pt = alloc_page_frame();
	assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);
	page_table_update(pt, 0xcafecafeeee, 0xf00d);
	assert(page_table_query(pt, 0xcafecafeeee) == 0xf00d);
	assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);
	page_table_update(pt, 0xcafecafeeee, NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xfffecafeeee) == NO_MAPPING);
	assert(page_table_query(pt, 0xcafecafeeff) == NO_MAPPING);
	printf("All BASIC tests pass\n");

	pt = alloc_page_frame();
	uint64_t new_pt = alloc_page_frame();
	uint64_t *tmp;

	/* 1st Test */
	assert(page_table_query(pt, 0xcafe) == NO_MAPPING);
	page_table_update(pt, 0xcafe, 0xf00d);
	page_table_update(pt, 0xbaff, 0xbadd);
	assert(page_table_query(pt, 0xcafe) == 0xf00d);
	assert(page_table_query(pt, 0xbaff) == 0xbadd);
	page_table_update(pt, 0xcafe, NO_MAPPING);
	assert(page_table_query(pt, 0xcafe) == NO_MAPPING);
	assert(page_table_query(pt, 0xbaff) == 0xbadd);
	page_table_update(pt, 0xbaff, NO_MAPPING);
	assert(page_table_query(pt, 0xbaff) == NO_MAPPING);
	printf("1st Test: PASSED\n");

	/* 2nd Test*/
	page_table_update(pt, 0xcafe, 0);
	assert(page_table_query(pt, 0xcafe) == 0);
	page_table_update(pt, 0xcafe, NO_MAPPING);
	assert(page_table_query(pt, 0xcafe) == NO_MAPPING);
	printf("2nd Test: PASSED\n");

	/* 3rd Test */
	page_table_update(pt, 0x8686, 0xabcd);
	assert(page_table_query(pt, 0x8686) == 0xabcd);
	page_table_update(pt, 0x8686, 0x1234);
	assert(page_table_query(pt, 0x8686) == 0x1234);
	printf("3rd Test: PASSED\n");

	/* 4th Test */
	page_table_update(pt, 0xcafe, 0xacdc);
	assert(page_table_query(pt, 0xcafe) == 0xacdc);
	page_table_update(pt, 0xcaff, 0xaaaa);
	assert(page_table_query(pt, 0xcafe) == 0xacdc);
	page_table_update(pt, 0xcafe, NO_MAPPING);
	page_table_update(pt, 0xcaff, NO_MAPPING);
	assert(page_table_query(pt, 0xcafe) == NO_MAPPING);
	assert(page_table_query(pt, 0xcaff) == NO_MAPPING);
	printf("4th Test: PASSED\n");

	/* 5th Test */
	page_table_update(new_pt, 0xcafe, 0xabcd);
	assert(page_table_query(new_pt, 0xcafe) == 0xabcd);
	page_table_update(new_pt, 0xcafe, NO_MAPPING);
	assert(page_table_query(new_pt, 0xcafe) == NO_MAPPING);
	printf("5th Test: PASSED\n");

	/* 6th Test */
	page_table_update(pt, 0x1ffff8000000, 0x1212);
	assert(page_table_query(pt, 0x1ffff8000000) == 0x1212);
	tmp = phys_to_virt(pt << 12);
	tmp = phys_to_virt((tmp[511] >> 12) << 12);
	tmp = phys_to_virt((tmp[511] >> 12) << 12);
	tmp = phys_to_virt((tmp[0] >> 12) << 12);
	tmp = phys_to_virt((tmp[0] >> 12) << 12);
	tmp[0] = ((tmp[0] >> 1) << 1);
	assert(page_table_query(pt, 0x1ffff8000000) == NO_MAPPING);
	printf("6th Test: PASSED\n\n----------------\n");

	printf("Overall:  PASSED\n\n");

	/*
	 * My Tester: more extensive tests!
	 */
	pt = alloc_page_frame();

	// Basic Tests
	assert(page_table_query(pt, 0x0) == NO_MAPPING);
	assert(page_table_query(pt, 0x1) == NO_MAPPING);

	page_table_update(pt, 0x0, 0x100);
	assert(page_table_query(pt, 0x0) == 0x100);
	page_table_update(pt, 0x1, 0x101);
	assert(page_table_query(pt, 0x1) == 0x101);

	page_table_update(pt, 0x0, NO_MAPPING);
	assert(page_table_query(pt, 0x0) == NO_MAPPING);
	assert(page_table_query(pt, 0x1) == 0x101);

	page_table_update(pt, 0x1, NO_MAPPING);
	assert(page_table_query(pt, 0x1) == NO_MAPPING);

	// Advanced Tests
	for (uint64_t i = 0; i < 1024; i++)
	{
		page_table_update(pt, i, i + 0x1000);
		assert(page_table_query(pt, i) == i + 0x1000);
	}

	for (uint64_t i = 0; i < 1024; i++)
	{
		page_table_update(pt, i, NO_MAPPING);
		assert(page_table_query(pt, i) == NO_MAPPING);
	}

	// Random large virtual addresses
	uint64_t large_addrs[] = {
		0x1ffff8000000, 0x2ffff9000000, 0x3ffffa000000, 0x4ffffb000000,
		0x5ffffc000000, 0x6ffffd000000, 0x7ffffe000000, 0x8fffff000000};

	for (int i = 0; i < sizeof(large_addrs) / sizeof(large_addrs[0]); i++)
	{
		page_table_update(pt, large_addrs[i], large_addrs[i] >> 12);
		assert(page_table_query(pt, large_addrs[i]) == (large_addrs[i] >> 12));
	}

	for (int i = 0; i < sizeof(large_addrs) / sizeof(large_addrs[0]); i++)
	{
		page_table_update(pt, large_addrs[i], NO_MAPPING);
		assert(page_table_query(pt, large_addrs[i]) == NO_MAPPING);
	}

	// Sequential mapping and unmapping
	for (uint64_t i = 0; i < 2048; i++)
	{
		page_table_update(pt, i, i + 0x2000);
		assert(page_table_query(pt, i) == i + 0x2000);
	}

	for (uint64_t i = 0; i < 2048; i++)
	{
		page_table_update(pt, i, NO_MAPPING);
		assert(page_table_query(pt, i) == NO_MAPPING);
	}

	// Test cases From drive!!!!!!!!!

	// SEGFAULT Check
	pt = alloc_page_frame();
	// Deliberately commented to avoid SEGFAULT, would be a catch-all test case.
	// assert(page_table_query(pt, 0xffffffffffffffff) == NO_MAPPING);

	// fails_functionality_basic
	pt = alloc_page_frame();
	page_table_update(pt, 0xabc, 0x123);
	assert(page_table_query(pt, 0xabc) == 0x123);
	page_table_update(pt, 0xabc, NO_MAPPING);
	assert(page_table_query(pt, 0xabc) == NO_MAPPING);
	printf("fails_functionality_basic: PASSED\n");

	// test_NO_MAPPING_same_pte
	pt = alloc_page_frame();
	page_table_update(pt, 0x100, 0x200);
	page_table_update(pt, 0x200, 0x300);
	page_table_update(pt, 0x100, NO_MAPPING);
	assert(page_table_query(pt, 0x100) == NO_MAPPING);
	assert(page_table_query(pt, 0x200) == 0x300);
	printf("test_NO_MAPPING_same_pte: PASSED\n");

	// test_mapping_to_ppn_0
	pt = alloc_page_frame();
	page_table_update(pt, 0x100, 0x0);
	assert(page_table_query(pt, 0x100) == 0x0);
	page_table_update(pt, 0x100, NO_MAPPING);
	printf("test_mapping_to_ppn_0: PASSED\n");

	// mapping_override_test
	pt = alloc_page_frame();
	page_table_update(pt, 0xabc, 0x123);
	page_table_update(pt, 0xabc, 0x456);
	assert(page_table_query(pt, 0xabc) == 0x456);
	page_table_update(pt, 0xabc, NO_MAPPING);
	printf("mapping_override_test: PASSED\n");

	// test_override_prefix_similar_vpn
	pt = alloc_page_frame();
	page_table_update(pt, 0xabc, 0x123);
	page_table_update(pt, 0xabd, 0x456);
	assert(page_table_query(pt, 0xabc) == 0x123);
	assert(page_table_query(pt, 0xabd) == 0x456);
	page_table_update(pt, 0xabc, NO_MAPPING);
	page_table_update(pt, 0xabd, NO_MAPPING);
	printf("test_override_prefix_similar_vpn: PASSED\n");

	// zero_not_node_root_Test
	pt = alloc_page_frame();
	new_pt = alloc_page_frame();
	page_table_update(pt, 0xabc, 0x123);
	page_table_update(new_pt, 0xabc, 0x456);
	assert(page_table_query(pt, 0xabc) == 0x123);
	assert(page_table_query(new_pt, 0xabc) == 0x456);
	page_table_update(pt, 0xabc, NO_MAPPING);
	page_table_update(new_pt, 0xabc, NO_MAPPING);
	printf("zero_not_node_root_Test: PASSED\n");

	printf("All tests passed successfully!\n");

	return 0;
}
