OBJ_DIR := ../../obj
H_DIR := ../../include

GEN_SRC := $(wildcard ./*.c)
OTHER_SRC := $(wildcard ../*.c)
SLV_SRC := $(wildcard ../solver/*.c)
Z3_SRC := $(wildcard ../solver/z3_solver/*.c)

GEN_OBJ := $(patsubst ./%.c, $(OBJ_DIR)/%.o, $(GEN_SRC))

CPPFLAGS := -I./$(H_DIR)



all: $(GEN_OBJ)

$(OBJ_DIR)/%.o: ./%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<



coverage:
	$(CC) $(COV_FLAGS) $(abspath $(GEN_SRC) $(SLV_SRC) $(Z3_SRC) $(OTHER_SRC))\
 -o ../../kenken -I$(abspath $(H_DIR)) $(LDFLAGS)
