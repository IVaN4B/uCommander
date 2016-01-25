CC=gcc
CFLAGS=`pkg-config --cflags --libs gtk+-3.0` -g -std=c11 -Wall
BUILDDIR=build
OBJECTS=src/main.o src/ucmd-app.o src/ucmd-dir-list.o
TARGET=$(BUILDDIR)/ucommander

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(BUILDDIR)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
