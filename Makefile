CC = gcc
VERSION = 2.0.3
CFLAGS = -Wall -Wextra -O2 -std=c11 -D_XOPEN_SOURCE=700 -DSEQDIA_VERSION="\"$(VERSION)\""

# Main binary
BIN_SRC = src/cli/main.c
BIN = seqdia

# Shared sources
COMMON_SRC = src/model/types.c \
             src/model/error.c \
             src/model/text_utils.c \
             src/lexer/lexer.c \
             src/parser/parser.c \
             src/renderer/canvas.c \
             src/renderer/renderer.c

# Test binary
TEST_SRC = tests/simple_test.c
TEST_BIN = tests/simple_test

OBJ_DIR = build
COMMON_OBJ = $(COMMON_SRC:%.c=$(OBJ_DIR)/%.o)
BIN_OBJ = $(BIN_SRC:%.c=$(OBJ_DIR)/%.o)
TEST_OBJ = $(TEST_SRC:%.c=$(OBJ_DIR)/%.o)

HEADERS = src/model/error.h src/model/types.h src/model/text_utils.h \
          src/lexer/lexer.h src/parser/parser.h \
          src/renderer/canvas.h src/renderer/renderer.h
FORMAT_SRC = $(BIN_SRC) $(COMMON_SRC) $(TEST_SRC) $(HEADERS)
TIDY_SRC = $(BIN_SRC) $(COMMON_SRC) $(TEST_SRC)

FIXTURE_INPUTS = tests/fixtures/notes.txt \
                 tests/fixtures/complex-options.txt \
                 tests/fixtures/complex-advanced.txt \
                 tests/fixtures/stress-large.txt

.PHONY: all clean test dump-fixtures format format-check lint lint-fix

all: $(BIN)

$(BIN): $(COMMON_OBJ) $(BIN_OBJ)
	$(CC) $(CFLAGS) $(COMMON_OBJ) $(BIN_OBJ) -o $(BIN)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BIN)
	@echo "Running tests..."
	@./$(TEST_BIN)

format:
	clang-format -i $(FORMAT_SRC)

format-check:
	clang-format --dry-run --Werror $(FORMAT_SRC)

lint:
	clang-tidy $(TIDY_SRC) -- $(CFLAGS)

lint-fix:
	clang-tidy -fix $(TIDY_SRC) -- $(CFLAGS)

$(TEST_BIN): $(COMMON_OBJ) $(TEST_OBJ)
	@mkdir -p $(dir $(TEST_BIN))
	$(CC) $(CFLAGS) $(COMMON_OBJ) $(TEST_OBJ) -o $(TEST_BIN)

dump-fixtures: $(BIN)
	@for file in $(FIXTURE_INPUTS); do \
	  base=$$(basename $$file .txt); \
	  ./$(BIN) -s ascii $$file > tests/fixtures/$$base.ascii.out; \
	  ./$(BIN) -s utf8 $$file > tests/fixtures/$$base.utf8.out; \
	done

clean:
	rm -f $(BIN) $(TEST_BIN)
	rm -rf $(OBJ_DIR)
