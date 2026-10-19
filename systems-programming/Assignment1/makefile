CC = gcc
CFLAGS = -Wall -std=c99 -g

.PHONY: all clean run_all

clean:
	rm -f memgrind \
		memtest_alignment memtest_overlap memtest_reuse memtest_coalesce memtest_oom \
		memtest_badfree_stack memtest_badfree_interior memtest_double_free memtest_leak \
		memtest_alignment_realmalloc memtest_overlap_realmalloc memtest_reuse_realmalloc \
		memtest_coalesce_realmalloc memtest_oom_realmalloc memtest_leak_realmalloc

all: memgrind \
	memtest_alignment memtest_overlap memtest_reuse memtest_coalesce memtest_oom \
	memtest_badfree_stack memtest_badfree_interior memtest_double_free memtest_leak \
	memtest_alignment_realmalloc memtest_overlap_realmalloc memtest_reuse_realmalloc \
	memtest_coalesce_realmalloc memtest_oom_realmalloc memtest_leak_realmalloc

memgrind: mymalloc.c memgrind.c mymalloc.h
	$(CC) $(CFLAGS) mymalloc.c memgrind.c -o memgrind

memtest_alignment: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_ALIGNMENT mymalloc.c memtest.c -o memtest_alignment

memtest_overlap: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_OVERLAP mymalloc.c memtest.c -o memtest_overlap

memtest_reuse: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_REUSE mymalloc.c memtest.c -o memtest_reuse

memtest_coalesce: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_COALESCE mymalloc.c memtest.c -o memtest_coalesce

memtest_oom: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_OOM mymalloc.c memtest.c -o memtest_oom

memtest_badfree_stack: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_BADFREE_STACK mymalloc.c memtest.c -o memtest_badfree_stack

memtest_badfree_interior: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_BADFREE_INTERIOR mymalloc.c memtest.c -o memtest_badfree_interior

memtest_double_free: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_DOUBLE_FREE mymalloc.c memtest.c -o memtest_double_free

memtest_leak: mymalloc.c memtest.c mymalloc.h
	$(CC) $(CFLAGS) -DTEST_LEAK mymalloc.c memtest.c -o memtest_leak

memtest_alignment_realmalloc: memtest.c
	$(CC) $(CFLAGS) -DREALMALLOC -DTEST_ALIGNMENT memtest.c -o memtest_alignment_realmalloc

memtest_overlap_realmalloc: memtest.c
	$(CC) $(CFLAGS) -DREALMALLOC -DTEST_OVERLAP memtest.c -o memtest_overlap_realmalloc

memtest_reuse_realmalloc: memtest.c
	$(CC) $(CFLAGS) -DREALMALLOC -DTEST_REUSE memtest.c -o memtest_reuse_realmalloc

memtest_coalesce_realmalloc: memtest.c
	$(CC) $(CFLAGS) -DREALMALLOC -DTEST_COALESCE memtest.c -o memtest_coalesce_realmalloc

memtest_oom_realmalloc: memtest.c
	$(CC) $(CFLAGS) -DREALMALLOC -DTEST_OOM memtest.c -o memtest_oom_realmalloc

memtest_leak_realmalloc: memtest.c
	$(CC) $(CFLAGS) -DREALMALLOC -DTEST_LEAK memtest.c -o memtest_leak_realmalloc

run_all: all
	@echo alignment; ./memtest_alignment
	@echo overlap; ./memtest_overlap
	@echo reuse; ./memtest_reuse
	@echo coalesce; ./memtest_coalesce
	@echo oom; ./memtest_oom
	@echo badfree_stack; ./memtest_badfree_stack; rc=$$?; test $$rc -eq 2
	@echo badfree_interior; ./memtest_badfree_interior; rc=$$?; test $$rc -eq 2
	@echo double_free; ./memtest_double_free; rc=$$?; test $$rc -eq 2
	@echo leak; ./memtest_leak
