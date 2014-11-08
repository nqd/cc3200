# invoke SourceDir generated makefile for app.pm4g
app.pm4g: .libraries,app.pm4g
.libraries,app.pm4g: package/cfg/app_pm4g.xdl
	$(MAKE) -f D:\CC32xx\SDK_0.9.0.0_RC\ti_rtos\ti_rtos_config/src/makefile.libs

clean::
	$(MAKE) -f D:\CC32xx\SDK_0.9.0.0_RC\ti_rtos\ti_rtos_config/src/makefile.libs clean

