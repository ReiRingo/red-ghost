.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment")
endif

include $(DEVKITARM)/gba_rules

GRIT ?= grit

TARGET := REDGHOST
BUILD  := build
GFXBUILD := gfx_build
AUDIOBUILD := audio_build

SOURCES  := source
INCLUDES := include
DATA     :=
MUSIC    := audio
GRAPHICS := gfx

ARCH := -mthumb -mthumb-interwork

CFLAGS := -g -Wall -O2 \
          -mcpu=arm7tdmi -mtune=arm7tdmi \
          $(ARCH)

CFLAGS += $(INCLUDE)

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS  := -g $(ARCH)
LDFLAGS  = -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS    := -lmm -lgba
LIBDIRS := $(LIBGBA)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT      := $(CURDIR)/$(TARGET)
export PROJECT_ROOT := $(CURDIR)
export DEPSDIR     := $(CURDIR)/$(BUILD)

export VPATH := $(CURDIR) \
                $(CURDIR)/$(GFXBUILD) \
                $(CURDIR)/$(AUDIOBUILD) \
                $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                $(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
                $(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

GFX_SPRITES := $(notdir $(wildcard $(GRAPHICS)/*.png))
GFX_BGS     := $(notdir $(wildcard $(GRAPHICS)/bgs/*.png))

export GFXOBJS  := $(GFX_SPRITES:.png=.o) $(GFX_BGS:.png=.o)

export GFXPNGS
export GFXOBJS

ifneq ($(strip $(MUSIC)),)
export AUDIOFILES := $(foreach file,$(notdir $(wildcard $(MUSIC)/*.*)),$(CURDIR)/$(MUSIC)/$(file))
BINFILES += soundbank.bin
endif

ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES_BIN := $(addsuffix .o,$(BINFILES)) $(GFXOBJS) shared_palette.o
export OFILES_SOURCES := $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD) \
                  -I$(CURDIR)/$(GFXBUILD) \
                  -I$(CURDIR)/$(AUDIOBUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

$(BUILD):
	@mkdir -p $(BUILD)
	@mkdir -p $(GFXBUILD)
	@mkdir -p $(AUDIOBUILD)
	@echo building...
	@if [ -n "$(GFX_BGS)" ]; then \
		cd $(GFXBUILD) && $(GRIT) $(addprefix ../$(GRAPHICS)/bgs/,$(GFX_BGS)) -gt -gB4 -m -mLs -ftc; \
	fi
	@if [ -n "$(GFX_SPRITES)" ]; then \
		cd $(GFXBUILD) && $(GRIT) $(addprefix ../$(GRAPHICS)/,$(GFX_SPRITES)) -gB4 -Mw 4 -Mh 4 -pS -Oshared_palette.c -Sshared -ftc; \
	fi
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo Cleaning...
	@rm -fr $(BUILD) $(GFXBUILD) $(AUDIOBUILD) $(TARGET).elf $(TARGET).gba

else

$(OUTPUT).gba: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)

$(OFILES_SOURCES): $(HFILES)

$(CURDIR)/../$(AUDIOBUILD)/soundbank.bin \
$(CURDIR)/../$(AUDIOBUILD)/soundbank.h: $(AUDIOFILES)
	@mkdir -p $(CURDIR)/../$(AUDIOBUILD)
	@mmutil $(AUDIOFILES) \
	    -o$(CURDIR)/../$(AUDIOBUILD)/soundbank.bin \
	    -h$(CURDIR)/../$(AUDIOBUILD)/soundbank.h

soundbank.bin.o: $(CURDIR)/../$(AUDIOBUILD)/soundbank.bin
	@$(bin2o) $<

%.bin.o %_bin.h: %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPSDIR)/*.d

endif
