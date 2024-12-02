#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

# Явно задаем путь к devkitpro
DEVKITPRO := /opt/devkitpro

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)

#---------------------------------------------------------------------------------
# Metadata for the application
#---------------------------------------------------------------------------------
APP_TITLE	:=	Switch BT Joy
APP_AUTHOR	:=	ev
APP_VERSION	:=	1.0.0
ICON		:=	icon.jpg

include $(DEVKITPRO)/libnx/switch_rules

TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source source/bluetooth
DATA		:=	data
INCLUDES	:=	include

ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS	:=	-O2 -ffunction-sections \
			$(ARCH) $(DEFINES) \
			$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			-I$(DEVKITPRO)/libnx/include \
			-I$(CURDIR)/$(BUILD)

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17

ASFLAGS	:=	$(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -lnx

LIBDIRS	:= $(PORTLIBS) $(LIBNX)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export LIBPATHS	:=	-L$(DEVKITPRO)/libnx/lib

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

all: $(BUILD) $(TARGET).nro

$(BUILD):
	@mkdir -p $@

$(TARGET).nro: $(BUILD)/$(TARGET).nro
	@cp $< $@

$(BUILD)/$(TARGET).nacp:
	@echo "creating $@"
	@nacptool --create "$(APP_TITLE)" "$(APP_AUTHOR)" "$(APP_VERSION)" $@

$(BUILD)/$(TARGET).nro: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).nacp
	@echo "creating $@"
	@elf2nro $< $@ --icon=$(CURDIR)/$(ICON) --nacp=$(BUILD)/$(TARGET).nacp $(NROFLAGS)

$(BUILD)/$(TARGET).elf: $(OFILES)
	@echo "linking $@"
	@$(LINK.cpp) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
# rules for assembling source files
#---------------------------------------------------------------------------------
$(BUILD)/%.o: %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ASFLAGS) -c $< -o $@

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).nro $(TARGET).nacp