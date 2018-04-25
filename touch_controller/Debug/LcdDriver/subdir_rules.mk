################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
LcdDriver/Crystalfontz128x128_ST7735.obj: ../LcdDriver/Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/Jon/somanyworkspaces/touch_controller" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/source/ti/devices/msp432p4xx/inc" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/driverlib/MSP432P4xx" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/GrLib/grlib" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/GrLib/fonts" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/LcdDriver" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/CMSIS/Include" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ARM_MATH_CM4 --define=__FPU_PRESENT=1 --define=_LINKAGE --define=_CODE_ACCESS="" --define=_DATA_ACCESS="" --define=TARGET_IS_MSP432P4XX --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="LcdDriver/Crystalfontz128x128_ST7735.d_raw" --obj_directory="LcdDriver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.obj: ../LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/Jon/somanyworkspaces/touch_controller" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/source/ti/devices/msp432p4xx/inc" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/driverlib/MSP432P4xx" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/GrLib/grlib" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/GrLib/fonts" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/LcdDriver" --include_path="C:/Users/Jon/somanyworkspaces/touch_controller/CMSIS/Include" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ARM_MATH_CM4 --define=__FPU_PRESENT=1 --define=_LINKAGE --define=_CODE_ACCESS="" --define=_DATA_ACCESS="" --define=TARGET_IS_MSP432P4XX --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.d_raw" --obj_directory="LcdDriver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


