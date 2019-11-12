OBJ_DIR := ../../obj
H_DIR := ../../include
Z3_DIR := z3_solver

SLV_SRC := $(wildcard ./*.c)
OTHER_SRC := $(wildcard ../*.c)
Z3_SRC := $(wildcard ./z3_solver/*.c)
GEN_SRC := $(wildcard ../generator/*.c)

SLV_OBJ := $(patsubst ./%.c, $(OBJ_DIR)/%.o, $(SLV_SRC))

CPPFLAGS := -I./$(H_DIR)



all: $(SLV_OBJ)
	@(cd $(Z3_DIR) && $(MAKE) $@)

$(OBJ_DIR)/%.o: ./%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<



coverage:
	$(CC) $(COV_FLAGS) $(abspath $(GEN_SRC) $(SLV_SRC) $(Z3_SRC) $(OTHER_SRC))\
 -o ../../kenken -I$(abspath $(H_DIR)) $(LDFLAGS)
