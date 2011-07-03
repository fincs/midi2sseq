# This Makefile was blatantly ripped off from the devkitPro project

.SUFFIXES:

TARGET  := $(shell basename $(CURDIR))
BUILD   := build
SOURCES := source

export CC    := gcc
export CXX   := g++
export STRIP := strip

CFLAGS   := -g0 -Wall -O2
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions

LDFLAGS := -g0

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export DEPSDIR := $(CURDIR)/$(BUILD)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

ifeq ($(strip $(CPPFILES)),)
	export LD := $(CC)
else
	export LD := $(CXX)
endif

export OFILES := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -fr $(BUILD) $(OUTPUT).exe

else

$(OUTPUT).exe: $(OFILES)

-include $(DEPSDIR)/*.d

%.exe:
	@echo linking...
	@$(LD) $(LDFLAGS) $(OFILES) -o $@
	@$(STRIP) --strip-all $@

%.o: %.c
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

endif
