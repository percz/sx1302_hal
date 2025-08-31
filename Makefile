### Environment constants

ARCH ?=
CROSS_COMPILE ?=
MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
GLOBAL_CFLAGS := -O3 -Wall -Wextra \
	-I$(MAKEFILE_DIR)inc \
	-I$(MAKEFILE_DIR) \
	-I$(MAKEFILE_DIR)libtools/inc \
	-I$(MAKEFILE_DIR)libloragw/inc \
	-I$(MAKEFILE_DIR)packet_forwarder/inc
export

### general build targets

.PHONY: all clean install install_conf libtools libloragw packet_forwarder util_net_downlink util_chip_id util_boot util_spectral_scan

all: libtools libloragw packet_forwarder util_net_downlink util_chip_id util_boot util_spectral_scan

print-makefile-dir:
	@echo MAKEFILE_DIR=$(MAKEFILE_DIR)

libtools:
	$(MAKE) all -e -C $(MAKEFILE_DIR)$@ CFLAGS='$(GLOBAL_CFLAGS)'

libloragw: libtools
	$(MAKE) all -e -C $(MAKEFILE_DIR)$@ CFLAGS='$(GLOBAL_CFLAGS)'

packet_forwarder: libloragw
	$(MAKE) all -e -C $(MAKEFILE_DIR)$@ CFLAGS='$(GLOBAL_CFLAGS)'

util_net_downlink: libtools
	$(MAKE) all -e -C $(MAKEFILE_DIR)$@ CFLAGS='$(GLOBAL_CFLAGS)'

util_chip_id: libloragw
	$(MAKE) all -e -C $(MAKEFILE_DIR)$@ CFLAGS='$(GLOBAL_CFLAGS)'

util_boot: libloragw
	$(MAKE) all -e -C $(MAKEFILE_DIR)$@ CFLAGS='$(GLOBAL_CFLAGS)'

util_spectral_scan: libloragw
	$(MAKE) all -e -C $(MAKEFILE_DIR)$@ CFLAGS='$(GLOBAL_CFLAGS)'

clean:
	$(MAKE) clean -e -C $(MAKEFILE_DIR)libtools
	$(MAKE) clean -e -C $(MAKEFILE_DIR)libloragw
	$(MAKE) clean -e -C $(MAKEFILE_DIR)packet_forwarder
	$(MAKE) clean -e -C $(MAKEFILE_DIR)util_net_downlink
	$(MAKE) clean -e -C $(MAKEFILE_DIR)util_chip_id
	$(MAKE) clean -e -C $(MAKEFILE_DIR)util_boot
	$(MAKE) clean -e -C $(MAKEFILE_DIR)util_spectral_scan

install:
	$(MAKE) install -e -C $(MAKEFILE_DIR)libloragw
	$(MAKE) install -e -C $(MAKEFILE_DIR)packet_forwarder
	$(MAKE) install -e -C $(MAKEFILE_DIR)util_net_downlink
	$(MAKE) install -e -C $(MAKEFILE_DIR)util_chip_id
	$(MAKE) install -e -C $(MAKEFILE_DIR)util_boot
	$(MAKE) install -e -C $(MAKEFILE_DIR)util_spectral_scan

install_conf:
	$(MAKE) install_conf -e -C $(MAKEFILE_DIR)packet_forwarder

### EOF
