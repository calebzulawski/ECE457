TARGET1 = test_tas
TARGET2 = test_sem
TARGET3 = test_fifo

LIBS   =
CC     = gcc
CFLAGS = -g -Wall -std=gnu99 -Wfatal-errors

.PHONY: default all clean

all: $(TARGET1) $(TARGET2)
default:all

T1_O = $(TARGET1).o
T2_O = $(TARGET2).o
T3_O = $(TARGET3).o
LIB_O = $(patsubst %.c, %.o, $(wildcard lib/*.c))
LIB_O += $(patsubst %.S, %.o, $(wildcard lib/*.S))
LIB_H = $(wildcard libs/*.h)

%.o: %.c $(LIB_H)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET1): $(T1_O) $(LIB_O)
	$(CC) $(T1_O) $(LIB_O) -Wall $(LIBS) -o $@

$(TARGET2): $(T2_O) $(LIB_O)
	$(CC) $(T2_O) $(LIB_O) -Wall $(LIBS) -o $@

$(TARGET3): $(T3_O) $(LIB_O)
	$(CC) $(T3_O) $(LIB_O) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

doc:
	a2ps lib/*.c lib/*.h *.c --output=../doc/sem-src.ps