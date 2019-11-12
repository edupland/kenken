TARGET := ../kenken
H_DIR := ../include
OBJ_DIR := ../obj
OUT_DIR := ../output
TEST_DIR := ../test
CHK_DIR := $(TEST_DIR)/check
COV_DIR := $(TEST_DIR)/coverage
GEN_DIR := generator
SLV_DIR := solver
Z3_DIR := $(SLV_DIR)/z3_solver

GEN_SRC := $(wildcard $(GEN_DIR)/*.c)
SLV_SRC := $(wildcard $(SLV_DIR)/*.c)
Z3_SRC := $(wildcard $(Z3_DIR)/*.c)
OTHER_SRC := $(wildcard ./*.c)

GEN_OBJ := $(patsubst $(GEN_DIR)/%.c, $(OBJ_DIR)/%.o, $(GEN_SRC))
SLV_OBJ := $(patsubst $(SLV_DIR)/%.c, $(OBJ_DIR)/%.o, $(SLV_SRC))
Z3_OBJ := $(patsubst $(Z3_DIR)/%.c, $(OBJ_DIR)/%.o, $(Z3_SRC))
OTHER_OBJ := $(patsubst ./%.c, $(OBJ_DIR)/%.o, $(OTHER_SRC))
OBJ_FILES := $(Z3_OBJ) $(SLV_OBJ) $(GEN_OBJ) $(OTHER_OBJ)

PROF_CFLAGS := -pg -no-pie
export COV_FLAGS := -fprofile-arcs -ftest-coverage

export CFLAGS := -Wall -Wextra -g -std=c11 -O3
CPPFLAGS := -I./$(H_DIR)
export LDFLAGS := -lm -lz3



all: $(OTHER_OBJ)
	@(cd $(GEN_DIR) && $(MAKE) $@)
	@(cd $(SLV_DIR) && $(MAKE) $@)
	$(CC) -o $(TARGET) $(OBJ_FILES) $(PROF_CFLAGS) $(LDFLAGS)

$(OBJ_DIR)/%.o: ./%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<



check:
	@(sh $(CHK_DIR)/check.sh)

coverage:
	$(CC) $(COV_FLAGS) $(abspath $(GEN_SRC) $(SLV_SRC) $(Z3_SRC) $(OTHER_SRC))\
 -o $(TARGET) -I$(abspath $(H_DIR)) $(LDFLAGS)




clean:
ifneq ($(findstring $(OUT_DIR), $(wildcard ../*)),)
	rm -r $(wildcard $(OUT_DIR))
endif
ifneq ($(findstring $(OBJ_DIR), $(wildcard ../*)),)
	rm -r $(wildcard $(OBJ_DIR))
endif
ifneq ($(findstring $(COV_DIR), $(wildcard ./$(TEST_DIR)/*)),)
	rm -r $(wildcard $(COV_DIR))
endif
ifneq ($(findstring $(TARGET), $(wildcard ../*)),)
	rm $(TARGET)
endif
