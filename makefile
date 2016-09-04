ESP_MAKE=$(HOME)/src/makeEspArduino
ESP_LIBS=$(HOME)/src/ArduinoEsp/libraries

ESP_ROOT=$(HOME)/src/ArduinoEsp

THIS_DIR := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
ROOT := $(THIS_DIR)

SKETCH=$(ROOT)/src/PhoxTailLight.ino

LIBS = $(ROOT)/libs \
	   $(ESP_LIBS)

include $(ESP_MAKE)/makeEspArduino.mk

