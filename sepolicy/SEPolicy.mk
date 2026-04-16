# Board specific SELinux policy variable definitions
SEPOLICY_PATH := device/huawei/compat/sepolicy

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    $(SEPOLICY_PATH)/common/private

SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += \
    $(SEPOLICY_PATH)/common/public

BOARD_VENDOR_SEPOLICY_DIRS += \
    $(SEPOLICY_PATH)/common/vendor

# kirin710
ifneq ($(filter kirin710,$(TARGET_BOARD_PLATFORM)),)
include device/huawei/compat/sepolicy/kirin710/sepolicy.mk
endif

# kirin970
ifneq ($(filter kirin970,$(TARGET_BOARD_PLATFORM)),)
include device/huawei/compat/sepolicy/kirin970/sepolicy.mk
endif

# kirin980
ifneq ($(filter kirin980,$(TARGET_BOARD_PLATFORM)),)
include device/huawei/compat/sepolicy/kirin980/sepolicy.mk
endif
