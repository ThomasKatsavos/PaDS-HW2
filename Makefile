export OMPI_CC=gcc
CC_SEQ = gcc
CFLAGS_SEQ = -std=c99 -O3 -Wall
SRCS_SEQ = labelprop_seq.c  
OBJS_SEQ = labelprop_seq.o mat_to_csr_seq.o main_seq.o
EXEC_SEQ = seq

#source files
MATTOCSR = mat_to_csr.c
MAIN = main.c

#libraries linkage
HEADERS = mat_to_csr.h bfs.h
LDFLAGS = -lmatio 
MATIO_DIR = ${MATIO_ROOT}

all: $(EXEC_SEQ) 

main_seq.o: $(MAIN) $(HEADERS)
	$(CC_SEQ) $(CFLAGS_SEQ) -c $< -o $@ -I$(MATIO_DIR)/include

mat_to_csr_seq.o: $(MATTOCSR) $(HEADERS)
	$(CC_SEQ) $(CFLAGS_SEQ) -c $< -o $@ -I$(MATIO_DIR)/include

%_seq.o: %_seq.c $(HEADERS)
	$(CC_SEQ) $(CFLAGS_SEQ) -c $< -o $@ -I$(MATIO_DIR)/include

$(EXEC_SEQ): $(OBJS_SEQ)
	$(CC_SEQ) $(CFLAGS_SEQ) -o $@ $^ -L$(MATIO_DIR)/lib -lmatio -lz


clean: 
	rm -f *.o $(EXEC_SEQ) 

.PHONY: all clean
