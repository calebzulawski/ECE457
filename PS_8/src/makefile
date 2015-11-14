TARGET1 = p1
TARGET2 = p2

LIBS   =
CC     = gcc
CFLAGS = -g -Wall -std=gnu99 -Wfatal-errors

.PHONY: default all clean

all: $(TARGET1) $(TARGET2)
default:all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET1): $(TARGET1).o
	$(CC) $(TARGET1).o -Wall $(LIBS) -o $@

$(TARGET2): $(T2_O) $(LIB_O)
	$(CC) $(T2_O) $(LIB_O) -Wall $(LIBS) -o $@

$(TARGET3): $(T3_O) $(LIB_O)
	$(CC) $(T3_O) $(LIB_O) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

doc:
	a2ps lib/*.c lib/*.h *.c --output=../doc/sem-src.ps