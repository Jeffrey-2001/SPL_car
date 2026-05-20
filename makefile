##############################################################################
# 目标配置
##############################################################################
TARGET = test01

# 调试版本（1=开启调试，0=发布版本）
DEBUG = 1

# 优化选项
OPT = -O0

# 编译输出目录
BUILD_DIR = build

##############################################################################
# 工具链配置
##############################################################################
PREFIX = arm-none-eabi-

# GCC路径配置（如果不在PATH中，请取消注释并设置实际路径）
# GCC_PATH = C:/gcc-arm-none-eabi/bin

ifdef GCC_PATH
    CC = $(GCC_PATH)/$(PREFIX)gcc
    AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
    CP = $(GCC_PATH)/$(PREFIX)objcopy
    SZ = $(GCC_PATH)/$(PREFIX)size
else
    CC = $(PREFIX)gcc
    AS = $(PREFIX)gcc -x assembler-with-cpp
    CP = $(PREFIX)objcopy
    SZ = $(PREFIX)size
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

##############################################################################
# 源文件配置（根据你的项目修改这部分！）
##############################################################################

# C源文件 - 使用wildcard自动搜集所有.c文件
# 注意：根据你的实际目录结构调整路径
C_SOURCES = \
$(wildcard ./lib/cmsis/*.c) \
$(wildcard ./lib/src/*.c) \
$(wildcard ./src/*.c)

# 汇编源文件（启动文件）
ASM_SOURCES = \
./lib/startup/startup_stm32f10x_md.s

##############################################################################
# 编译选项
##############################################################################

# MCU型号配置（根据你的STM32型号修改）
# Cortex-M0: -mcpu=cortex-m0
# Cortex-M3: -mcpu=cortex-m3
# Cortex-M4: -mcpu=cortex-m4
CPU = -mcpu=cortex-m3

# 浮点单元（Cortex-M0/M3没有FPU，注释掉即可）
# FPU = -mfpu=fpv4-sp-d16
# FLOAT-ABI = -mfloat-abi=hard

MCU = $(CPU) -mthumb

# 宏定义（根据你的芯片型号修改）
# STM32F10X_MD: 中等容量产品
# STM32F10X_HD: 高容量产品
# STM32F10X_CL: 互联型产品
C_DEFS = \
-DSTM32F10X_MD \
-DUSE_STDPERIPH_DRIVER

# 头文件路径（根据你的项目结构调整）
C_INCLUDES = \
-Ilib/cmsis \
-Ilib/inc \
-Isrc \
-Isrc/inc

# 汇编选项
AS_DEFS = 

# 汇编头文件路径
AS_INCLUDES = 

# 编译标志
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
    CFLAGS += -g -gdwarf-2
endif

# 生成依赖信息
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# 汇编标志
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

##############################################################################
# 链接配置
##############################################################################

# 链接脚本（根据芯片和项目修改）
LDSCRIPT = STM32F103C8Tx_FLASH.ld

# 库文件
LIBS = -lc -lm -lnosys
LIBDIR = 

LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

##############################################################################
# 自动生成对象文件列表
##############################################################################

# C对象文件
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# 汇编对象文件
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

##############################################################################
# 编译规则
##############################################################################

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# 创建build目录
$(BUILD_DIR):
	mkdir $@

# 编译C文件
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

# 编译汇编文件
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

# 链接
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

# 生成hex文件
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

# 生成bin文件
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

##############################################################################
# 清理
##############################################################################

# Windows和Linux兼容的删除命令
ifeq ($(OS),Windows_NT)
    RM = rmdir /S /Q
else
    RM = rm -rf
endif

clean:
	-$(RM) $(BUILD_DIR)

##############################################################################
# 依赖文件包含
##############################################################################
-include $(wildcard $(BUILD_DIR)/*.d)

##############################################################################
# 烧录（可选）
##############################################################################
# 使用ST-Link烧录（需要安装openocd）
flash: all
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(BUILD_DIR)/$(TARGET).bin 0x8000000 verify reset exit"

# 使用J-Link烧录
flash_jlink: all
	JLinkExe -device STM32F103C8 -if SWD -speed 4000 -autoconnect 1 -CommanderScript flash.jlink

.PHONY: all clean flash flash_jlink