TARGET = cilk_mpi

SRCS = mat_to_csr_ompi.c  mpimain.c  parall_cilk_mpi.c
MATIO_DIR = ${MATIO_ROOT}
OBJS = $(SRCS:.c=.o)
CC = mpicc

export OMPI_CC=/home/thomas1/opt/opencilk/bin/clang
#/mnt/apps/custom/OpenCilk/v2.0/bin/clang
CFLAGS =  -O3 -Wno-pointer-sign
LDFLAGS = -lmatio

.PHONY: all
all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) -fopencilk -L$(MATIO_DIR)/lib -lmatio -lz

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) -fopencilk -I$(MATIO_DIR)/include

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)

