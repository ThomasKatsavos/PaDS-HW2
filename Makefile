#Sequential version compiler settings

CC_SEQ = gcc
CFLAGS_SEQ = -std=c99 -O3 -Wall
SRCS_SEQ = labelprop_seq.c  
OBJS_SEQ = labelprop_seq.o mat_to_csr_seq.o main_seq.o
EXEC_SEQ = seq

#OpenCilk version compiler settings

CC_CILK = ~/opt/opencilk/bin/clang
CFLAGS_CILK = -fopencilk -fcilktool=cilkscale -O3
SRCS_CILK = labelprop_ocilk.c  
OBJS_CILK = labelprop_ocilk.o mat_to_csr_cilk.o main_cilk.o
EXEC_CILK = cilk

#Common source files
MATTOCSR = mat_to_csr.c
MAIN = main.c

#Common libraries linkage
HEADERS = mat_to_csr.h bfs.h
LDFLAGS = -lmatio 

#Names for different compilation commands


all: $(EXEC_SEQ) $(EXEC_CILK)

main_seq.o: $(MAIN) $(HEADERS)
	$(CC_SEQ) $(CFLAGS_SEQ) -c $< -o $@

mat_to_csr_seq.o: $(MATTOCSR) $(HEADERS)
	$(CC_SEQ) $(CFLAGS_SEQ) -c $< -o $@

%_seq.o: %_seq.c $(HEADERS)
	$(CC_SEQ) $(CFLAGS_SEQ) -c $< -o $@

$(EXEC_SEQ): $(OBJS_SEQ)
	$(CC_SEQ) $(CFLAGS_SEQ) -o $@ $^ $(LDFLAGS)



main_cilk.o: $(MAIN)
	$(CC_CILK) $(CFLAGS_CILK) -c $< -o $@

mat_to_csr_cilk.o: $(MATTOCSR)
	$(CC_CILK) $(CFLAGS_CILK) -c $< -o $@

%cilk.o: %cilk.c $(HEADERS)
	$(CC_CILK) $(CFLAGS_CILK) -c $< -o $@

$(EXEC_CILK): $(OBJS_CILK)
	$(CC_CILK) $(CFLAGS_CILK) -o $@ $^ $(LDFLAGS)



clean: 
	rm -f *.o $(EXEC_SEQ) $(EXEC_CILK)

.PHONY: all clean
