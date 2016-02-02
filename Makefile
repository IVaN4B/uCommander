CC=gcc
CFLAGS=`pkg-config --cflags --libs gtk+-3.0` -DGDK_VERSION_MIN_REQIRED=GDK_VERSION_3_12 -g -std=c11 -Wall
BUILDDIR=build
OBJECTS=src/main.o src/ucmd-app.o src/ucmd-dir-list.o src/ucmd-dir-view.o src/ucmd-file-job.o
TARGET=$(BUILDDIR)/ucommander

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(BUILDDIR)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
