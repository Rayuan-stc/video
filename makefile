CROSS_COMPILE ?= arm-fullhanv3-linux-uclibcgnueabi-

APP_NAME := videocatch
BUILD_DIR := build

PLATFORM_DIR := platform/fh8862
CAPTURE_DIR := modules/capture
COMMON_DIR := modules/common
APP_DIR := app

CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip

CFLAGS := -Wall -Werror -fno-aggressive-loop-optimizations
CFLAGS += -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable
CFLAGS += -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
CFLAGS += -ffunction-sections -fdata-sections -ftree-vectorize -fPIC

INC_DIRS := \
	-I$(APP_DIR) \
	-I$(COMMON_DIR)/inc \
	-I$(CAPTURE_DIR)/inc \
	-I$(PLATFORM_DIR)/include \
	-I$(PLATFORM_DIR)/include/types \
	-I$(PLATFORM_DIR)/include/dsp \
	-I$(PLATFORM_DIR)/include/dsp_ext \
	-I$(PLATFORM_DIR)/include/isp \
	-I$(PLATFORM_DIR)/include/isp_ext \
	-I$(PLATFORM_DIR)/include/mpp \
	-I$(PLATFORM_DIR)/include/vicap

CFLAGS += $(INC_DIRS)

LIBS_DIR := $(PLATFORM_DIR)/lib/static

FH_MPP_LIBS := \
	$(LIBS_DIR)/libdsp.a \
	$(LIBS_DIR)/libdbi.a \
	$(LIBS_DIR)/libvb_mpi.a \
	$(LIBS_DIR)/libvmm.a \
	$(LIBS_DIR)/libmipi.a \
	$(LIBS_DIR)/libimx415_mipi.a \
	$(LIBS_DIR)/libisp.a \
	$(LIBS_DIR)/libispcore.a \
	$(LIBS_DIR)/libadvapi_osd.a \
	$(LIBS_DIR)/libadvapi.a

LDLIBS := $(FH_MPP_LIBS) -lstdc++ -lpthread -lm -lrt -ldl -rdynamic

SRCS := \
	$(APP_DIR)/main.c \
	$(COMMON_DIR)/src/rtc_time.c \
	$(CAPTURE_DIR)/src/capture.c \
	$(CAPTURE_DIR)/src/isp_pipeline.c \
	$(CAPTURE_DIR)/src/isp.c \
	$(CAPTURE_DIR)/src/media_buffer.c \
	$(CAPTURE_DIR)/src/osd.c \
	$(CAPTURE_DIR)/src/stream.c \
	$(CAPTURE_DIR)/src/venc.c \
	$(CAPTURE_DIR)/src/vpu.c \
	$(CAPTURE_DIR)/src/libdmc.c \
	$(CAPTURE_DIR)/src/libdmc_pes.c \
	$(CAPTURE_DIR)/src/libdmc_record_raw.c \
	$(CAPTURE_DIR)/src/libpes.c \
	$(CAPTURE_DIR)/src/sensor.c

OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean

all: $(BUILD_DIR)/$(APP_NAME)

$(BUILD_DIR)/$(APP_NAME): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)
	$(STRIP) $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
