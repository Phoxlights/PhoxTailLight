THIS_DIR := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
ROOT := $(THIS_DIR)

ESP_MAKE=$(HOME)/src/makeEspArduino
ESP_LIBS=$(HOME)/src/ArduinoEsp/libraries

# makeEspArduino overrides
BUILD_DIR = $(ROOT)/tmp
FLASH_DEF=4M1M
ESP_ROOT=$(HOME)/src/ArduinoEsp
SKETCH=$(ROOT)/src/PhoxTailLight.ino
LIBS = $(ROOT)/libs \
	   $(ESP_LIBS)

include $(ESP_MAKE)/makeEspArduino.mk

build: all
	cp $(MAIN_EXE) build

DEVICE = 0

send_message:
	cat $(MESSAGE) | netcat $(MESSAGE_RECIPIENT_IP) $(MESSAGE_SERVER_PORT) 

listen:
	picocom --imap lfcrlf -e c -b 115200 /dev/ttyUSB$(DEVICE) 

.PHONY: ota_deploy send_message listen
