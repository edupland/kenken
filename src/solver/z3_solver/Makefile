CC := gcc
OBJ_DIR := ../../../obj
H_DIR := ../../../include

Z3_SRC := $(wildcard ./*.c)
SLV_SRC := $(wildcard ../*.c)
OTHER_SRC := $(wildcard ../../*.c)
GEN_SRC := $(wildcard ../../generator/*.c)

Z3_OBJ := $(patsubst ./%.c, $(OBJ_DIR)/%.o, $(Z3_SRC))

CPPFLAGS := -I./$(H_DIR)



all: $(Z3_OBJ)

$(OBJ_DIR)/%.o: ./%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<



coverage:
	$(CC) $(COV_FLAGS) $(abspath $(GEN_SRC) $(SLV_SRC) $(Z3_SRC) $(OTHER_SRC))\
 -o ../../../kenken -I$(abspath $(H_DIR)) $(LDFLAGS)
