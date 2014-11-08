#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/ccsv6/ccs_base;C:/ti/tirtos_simplelink_2_01_00_03/packages;C:/ti/tirtos_simplelink_2_01_00_03/products/bios_6_40_03_39/packages;C:/ti/tirtos_simplelink_2_01_00_03/products/uia_2_00_01_34/packages;C:/ti/CC3200SDK_1.0.0/cc3200-sdk/ti_rtos/ti_rtos_config/ccs/.config
override XDCROOT = c:/ti/xdctools_3_30_03_47_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/ccsv6/ccs_base;C:/ti/tirtos_simplelink_2_01_00_03/packages;C:/ti/tirtos_simplelink_2_01_00_03/products/bios_6_40_03_39/packages;C:/ti/tirtos_simplelink_2_01_00_03/products/uia_2_00_01_34/packages;C:/ti/CC3200SDK_1.0.0/cc3200-sdk/ti_rtos/ti_rtos_config/ccs/.config;c:/ti/xdctools_3_30_03_47_core/packages;..
HOSTOS = Windows
endif
