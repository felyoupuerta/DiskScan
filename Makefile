# ============================================================
#  DiskScan — Makefile
# ============================================================

CC   := gcc
STD  := -std=c11
WARN := -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
        -Wstrict-prototypes -Wwrite-strings -Wpointer-arith -Wvla
DEFS := -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
INC  := -Isrc
DEP  := -MMD -MP

BASE := $(STD) $(WARN) $(DEFS) $(INC) $(DEP)

# Se añade -static a LDFLAGS para incrustar glibc completa
STATIC_LDFLAGS := -static

REL_CFLAGS  := -O2 -DNDEBUG
DBG_CFLAGS  := -O0 -g3
DBG_LDLIBS  := 

BIN := dsk

# ---- Descubrimiento automático de fuentes ----
SRC      := $(wildcard src/*.c)
MAIN_SRC := src/main.c
LIB_SRC  := $(filter-out $(MAIN_SRC),$(SRC))

REL_OBJ     := $(SRC:src/%.c=build/rel/%.o)
DBG_OBJ     := $(SRC:src/%.c=build/dbg/%.o)
DBG_LIB_OBJ := $(LIB_SRC:src/%.c=build/dbg/%.o)

TEST_SRC := $(wildcard tests/test_%.c)
TEST_BIN := $(TEST_SRC:tests/%.c=build/%)

# ---- Objetivo por defecto ----
ifneq ($(wildcard src/main.c),)
all: $(BIN)
else
all: test
	@echo ">> Todavía no existe src/main.c: solo se han construido los tests."
endif

# ---- Binario release (Estático con glibc incluida) ----
$(BIN): $(REL_OBJ)
	$(CC) $(STATIC_LDFLAGS) $(REL_OBJ) -o $@

build/rel/%.o: src/%.c | build/rel
	$(CC) $(BASE) $(REL_CFLAGS) -c $< -o $@

# ---- Regla explícita para compilación estática ----
static: clean $(BIN)
	@echo ">> Binario $(BIN) compilado estáticamente con glibc."

# ---- Objetos de depuración ----
build/dbg/%.o: src/%.c | build/dbg
	$(CC) $(BASE) $(DBG_CFLAGS) -c $< -o $@

debug: $(DBG_OBJ)
	$(CC) $(DBG_OBJ) -o $(BIN)-debug $(DBG_LDLIBS)

# ---- Tests ----
build/test_%: tests/test_%.c $(DBG_LIB_OBJ) | build
	$(CC) $(BASE) $(DBG_CFLAGS) $< $(DBG_LIB_OBJ) -o $@ $(DBG_LDLIBS)

test: $(TEST_BIN)
	@echo "=========== TESTS ==========="
	@fallos=0; \
	for t in $(TEST_BIN); do \
		printf '%-28s ' "$$(basename $$t)"; \
		if ASAN_OPTIONS=detect_leaks=1 ./$$t > /tmp/ds_test.log 2>&1; then \
			echo "PASS"; \
		else \
			echo "FAIL"; cat /tmp/ds_test.log; fallos=1; \
		fi; \
	done; \
	exit $$fallos

# ---- Utilidades ----
run: $(BIN)
	./$(BIN) $(ARGS)

valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes ./$(BIN)-debug $(ARGS)

fixture:
	@rm -rf /tmp/dtest
	@mkdir -p /tmp/dtest/a /tmp/dtest/b /tmp/dtest/sinperm
	@dd if=/dev/urandom of=/tmp/dtest/a/real bs=1M count=10 status=none
	@truncate -s 1G /tmp/dtest/a/sparse
	@ln /tmp/dtest/a/real /tmp/dtest/b/enlace
	@ln -s /tmp/dtest/a   /tmp/dtest/b/sym
	@chmod 000 /tmp/dtest/sinperm
	@echo "Fixture en /tmp/dtest:"
	@du -sh  /tmp/dtest 2>/dev/null || true
	@du -sh --apparent-size /tmp/dtest 2>/dev/null || true

build build/rel build/dbg:
	mkdir -p $@

clean:
	rm -rf build $(BIN) $(BIN)-debug

distclean: clean
	rm -rf /tmp/dtest

.PHONY: all static debug test run valgrind fixture clean distclean
.DELETE_ON_ERROR:

-include $(wildcard build/rel/*.d build/dbg/*.d)