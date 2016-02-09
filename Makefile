# Ultimate Makefile v1.0 (C) 2016 Ivan Chebykin
# NOTE: GNU Make only

# Variables--------------------------------------------------------------------
PROJECT=ucommander

CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0` -DGDK_VERSION_MIN_REQIRED=GDK_VERSION_3_12 -g -std=c11 -Wall
LDFLAGS=`pkg-config --libs gtk+-3.0`

OBJEXT=o
SRCEXT=c

SRCDIR=src
BUILDDIR=build
OBJDIR=obj

OBJPATH=$(BUILDDIR)/$(OBJDIR)
TARGET=$(BUILDDIR)/$(PROJECT)

SOURCES=$(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS=$(patsubst $(SRCDIR)/%,$(OBJPATH)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

# Targets----------------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILDDIR)/$(OBJDIR)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(OBJPATH)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

cleanobj:
	rm -rf $(OBJPATH)

remake: clean all

.PHONY: all clean cleanobj remake
