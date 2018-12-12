EXEC = hw4_mm_test
TARGETS = $(EXEC)
CC := gcc
override CFLAGS += -O3 -Wall
OBJS = hw4_mm_test.o ./lib/hw_malloc.o
SUBDIR = ./lib
GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) $(TARGETS)

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

hw4_mm_test.o: %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

./lib/hw_malloc.o:
	$(MAKE) -C $(SUBDIR)

.PHONY: clean
clean:
	rm -rf *.o $(EXEC)
	$(MAKE) -C $(SUBDIR) clean