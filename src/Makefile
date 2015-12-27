CC=gcc
CFLAGS=`pkg-config --cflags --libs gtk+-3.0`
BUILDDIR=../build
OBJECTS=main.o
TARGET=$(BUILDDIR)/ucommander

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(BUILDDIR)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
