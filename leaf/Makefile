PROJECT = ToSaMeshLeaf
LIBSOURCES = new MiniArduino rfm70 ToSaMeshAppMessageQueue ToSaMeshMeshMessageBacklog ToSaMeshMeshMessageTracker ToSaMeshNode ToSaMeshRoutingTable ToSaMeshTransceiverBase ToSaMeshTransceiverRfm70 ToSaMeshLeafBase CapacitiveSensor
LIBHEADERS = pins rfm70reg ToSaMeshProtocol ToSaMeshUtils ToSaMeshNodeType ToSaMeshNodeConfig

DB_HWTYPE = 2
DB_HWVERSION = 1
DB_FWVERSION = 1

FINALOUT = ../master/firmware/$(DB_HWTYPE)_$(DB_HWVERSION)_$(DB_FWVERSION)_$(PROJECT)

MCU = atmega328p
CLK = 16000000L
#CORE = arduino
#VARIANT = standard

#CORESOURCES_C = hooks WInterrupts wiring wiring_analog wiring_digital wiring_pulse wiring_shift
#CORESOURCES_LIBC = malloc realloc
#CORESOURCES_CPP = CDC HardwareSerial HID IPAddress main new Print Stream Tone USBCore WMath WString

ifeq ($(OS),Windows_NT)
	BINPATH = ../arduino/hardware/tools/avr/bin/
#	COREPATH = ../arduino/hardware/arduino/avr/cores/$(CORE)
#	VARIANTPATH = ../arduino/hardware/arduino/avr/variants/$(VARIANT)
else
	UNAME_S := $(shell uname -s)
	UNAME_P := $(shell uname -p)
	UNAME_M := $(shell uname -m)
	ifeq ($(UNAME_S),Linux)
		ifeq ($(UNAME_P),x86_64)
			BINPATH = ../arduino/bin-linux64/bin/
			COREPATH = ../arduino/avr/cores/$(CORE)
			VARIANTPATH = ../arduino/avr/variants/$(VARIANT)
		else ifneq ($(filter %86,$(UNAME_P)),)
			BINPATH = ../arduino/bin-linux32/bin/
			COREPATH = ../arduino/avr/cores/$(CORE)
			VARIANTPATH = ../arduino/avr/variants/$(VARIANT)
		else ifneq ($(filter arm%,$(UNAME_M)),)
			BINPATH = /usr/bin/
			#COREPATH = /usr/share/arduino/hardware/arduino/cores/$(CORE)
			COREPATH = /home/pi/ToSaMesh/arduino/avr/cores/$(CORE)
			#VARIANTPATH = /usr/share/arduino/hardware/arduino/variants/$(VARIANT)
			VARIANTPATH = /home/pi/ToSaMesh/arduino/avr/variants/$(VARIANT)
		endif
	endif
endif

PWD = $(shell pwd)
SOURCEPATH = source
BUILDPATH = build
LIBPATH = ../lib
OUTPATH = out

#ARDUINODEFS = -DARDUINO=153 -DARDUINO_AVR_PRO -DARDUINO_ARCH_AVR

#CFLAGS = -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=$(MCU) -DF_CPU=$(CLK) $(ARDUINODEFS) -I$(COREPATH) -I$(VARIANTPATH)
CFLAGS = -c -g -Os -w -ffunction-sections -fdata-sections -MMD -mmcu=$(MCU) -DF_CPU=$(CLK)
#CPPFLAGS = -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=$(MCU) -DF_CPU=$(CLK) $(ARDUINODEFS) -I$(COREPATH) -I$(VARIANTPATH)
CPPFLAGS = -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -MMD -mmcu=$(MCU) -DF_CPU=$(CLK)
LDFLAGS = -Os -Wl,--gc-sections -mmcu=$(MCU)

all: clean source out

clean:
	rm -rf $(OUTPATH)
#	rm -rf $(SOURCEPATH)
	rm -rf $(BUILDPATH)/lib
	rm -f $(BUILDPATH)/$(PROJECT).*

#source: $(SOURCEPATH)/$(PROJECT).h $(SOURCEPATH)/$(PROJECT).cpp $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBSOURCES))) $(addprefix $(SOURCEPATH)/, $(addsuffix .cpp, $(LIBSOURCES))) $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBHEADERS)))
source: $(SOURCEPATH)/$(PROJECT).h $(SOURCEPATH)/$(PROJECT).cpp $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBSOURCES))) $(addprefix $(SOURCEPATH)/, $(addsuffix .cpp, $(LIBSOURCES))) $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBHEADERS)))

$(SOURCEPATH)/%.h: %.h
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.cpp: %.cpp
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.h: $(LIBPATH)/%.h
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.cpp: $(LIBPATH)/%.cpp
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(BUILDPATH)/$(PROJECT).o: $(SOURCEPATH)/$(PROJECT).cpp
	mkdir -p $(BUILDPATH)
	$(BINPATH)avr-g++ $(CPPFLAGS) -I$(SOURCEPATH) $< -o $@

$(BUILDPATH)/lib/%.o: $(SOURCEPATH)/%.cpp
	mkdir -p $(BUILDPATH)/lib
	$(BINPATH)avr-g++ $(CPPFLAGS) -I$(SOURCEPATH) $< -o $@

#$(BUILDPATH)/%.o: $(COREPATH)/%.cpp
#	mkdir -p $(BUILDPATH)
#	$(BINPATH)avr-g++ $(CPPFLAGS) $< -o $@

#$(BUILDPATH)/%.o: $(COREPATH)/%.c
#	mkdir -p $(BUILDPATH)
#	$(BINPATH)avr-gcc $(CFLAGS) $< -o $@

#$(BUILDPATH)/%.o: $(COREPATH)/avr-libc/%.c
#	mkdir -p $(BUILDPATH)
#	$(BINPATH)avr-gcc $(CFLAGS) $< -o $@

#$(BUILDPATH)/core.a: $(addprefix $(BUILDPATH)/, $(addsuffix .o, $(CORESOURCES_LIBC))) $(addprefix $(BUILDPATH)/, $(addsuffix .o, $(CORESOURCES_C))) $(addprefix $(BUILDPATH)/, $(addsuffix .o, $(CORESOURCES_CPP)))
#	$(BINPATH)avr-ar rcs $(BUILDPATH)/core.a $(addprefix $(BUILDPATH)/, $(addsuffix .o, $(CORESOURCES_LIBC)))
#	$(BINPATH)avr-ar rcs $(BUILDPATH)/core.a $(addprefix $(BUILDPATH)/, $(addsuffix .o, $(CORESOURCES_C)))
#	$(BINPATH)avr-ar rcs $(BUILDPATH)/core.a $(addprefix $(BUILDPATH)/, $(addsuffix .o, $(CORESOURCES_CPP)))

#$(OUTPATH)/$(PROJECT).elf: $(BUILDPATH)/$(PROJECT).o $(addprefix $(BUILDPATH)/lib/, $(addsuffix .o, $(LIBSOURCES))) $(BUILDPATH)/core.a
$(OUTPATH)/$(PROJECT).elf: $(BUILDPATH)/$(PROJECT).o $(addprefix $(BUILDPATH)/lib/, $(addsuffix .o, $(LIBSOURCES)))
	mkdir -p $(OUTPATH)
#	$(BINPATH)avr-gcc $(LDFLAGS) -o $@ $(BUILDPATH)/$(PROJECT).o $(addprefix $(BUILDPATH)/lib/, $(addsuffix .o, $(LIBSOURCES))) $(BUILDPATH)/core.a -L$(BUILDPATH) -lm
	$(BINPATH)avr-gcc $(LDFLAGS) -o $@ $(BUILDPATH)/$(PROJECT).o $(addprefix $(BUILDPATH)/lib/, $(addsuffix .o, $(LIBSOURCES))) -L$(BUILDPATH) -lm

$(OUTPATH)/$(PROJECT).eep: $(OUTPATH)/$(PROJECT).elf
	$(BINPATH)avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $@

$(OUTPATH)/$(PROJECT).hex: $(OUTPATH)/$(PROJECT).elf
	$(BINPATH)avr-objcopy -O ihex -R .eeprom $< $@ 

out: $(OUTPATH)/$(PROJECT).elf $(OUTPATH)/$(PROJECT).eep $(OUTPATH)/$(PROJECT).hex
	$(BINPATH)avr-size $(OUTPATH)/$(PROJECT).elf
#	cp -f $(OUTPATH)/$(PROJECT).elf $(FINALOUT).elf
#	cp -f $(OUTPATH)/$(PROJECT).eep $(FINALOUT).eep
	cp -f $(OUTPATH)/$(PROJECT).hex $(FINALOUT).hex
