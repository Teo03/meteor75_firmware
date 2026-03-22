-include $(ROOT)/oot_pre.mk
ifneq ($(TARGET),SITL)
CFLAGS += -Wno-error=enum-int-mismatch
endif
CFLAGS += -D RL_TOOLS_BETAFLIGHT_VERSION_4_5
