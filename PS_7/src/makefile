TARGET1 = test7
TARGET2 = fifo7

LIBS   =
CC     = gcc
CFLAGS = -g -Wall -std=gnu99 -Wfatal-errors

.PHONY: default all clean

all: $(TARGET1) $(TARGET2)
default:all

T1_O = $(TARGET1).o
T2_O = $(TARGET2).o
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

clean:
	-rm -f *.o
	-rm -f $(TARGET)

doc:
	a2ps *.c *.h *.sh *.txt --output=../doc/$(TARGET)-src.ps