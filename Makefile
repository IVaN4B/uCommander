CC=gcc
CFLAGS=`pkg-config --cflags --libs gtk+-3.0` -g -std=c11
BUILDDIR=build
OBJECTS=src/main.o src/ucmd-app.o
TARGET=$(BUILDDIR)/ucommander

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(BUILDDIR)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
