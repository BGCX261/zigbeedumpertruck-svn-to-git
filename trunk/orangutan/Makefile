DEVICE = atmega328p
AVRDUDE_DEVICE = m328p

CFLAGS=-g -Wall -mcall-prologues -mmcu=$(DEVICE) -Os
CC=avr-gcc
OBJ2HEX=avr-objcopy 
LDFLAGS=-Wl,-gc-sections -lpololu_$(DEVICE) -Wl,-relax

PORT = /dev/ttyUSB1
PROGRAMMER=stk500v2
AVRDUDE=avrdude
TARGET=main
OBJECT_FILES=main.o ds1820.o control.o

all: $(TARGET).hex

clean:
	rm -f *.o *.hex *.obj *.hex

%.hex: %.obj
	$(OBJ2HEX) -R .eeprom -O ihex $< $@

test.o: test.c

%.obj: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(OBJECT_FILES) $(LDFLAGS) -o $@

program: $(TARGET).hex
	$(AVRDUDE) -p $(AVRDUDE_DEVICE) -c $(PROGRAMMER) -P $(PORT) -U flash:w:$(TARGET).hex

