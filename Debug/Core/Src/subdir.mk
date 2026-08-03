################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/AI_Normalisation.c \
../Core/Src/AO_Normalisation.c \
../Core/Src/Block_RTC.c \
../Core/Src/Block_Sifu.c \
../Core/Src/Block_Synhro.c \
../Core/Src/DB_Constants.c \
../Core/Src/DB_Main.c \
../Core/Src/DB_Parameters.c \
../Core/Src/DI_Block.c \
../Core/Src/DO_Block.c \
../Core/Src/Reg_System.c \
../Core/Src/event_log.c \
../Core/Src/main.c \
../Core/Src/stm32f7xx_hal_msp.c \
../Core/Src/stm32f7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f7xx.c \
../Core/Src/trace_buffer.c \
../Core/Src/web_server.c 

OBJS += \
./Core/Src/AI_Normalisation.o \
./Core/Src/AO_Normalisation.o \
./Core/Src/Block_RTC.o \
./Core/Src/Block_Sifu.o \
./Core/Src/Block_Synhro.o \
./Core/Src/DB_Constants.o \
./Core/Src/DB_Main.o \
./Core/Src/DB_Parameters.o \
./Core/Src/DI_Block.o \
./Core/Src/DO_Block.o \
./Core/Src/Reg_System.o \
./Core/Src/event_log.o \
./Core/Src/main.o \
./Core/Src/stm32f7xx_hal_msp.o \
./Core/Src/stm32f7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f7xx.o \
./Core/Src/trace_buffer.o \
./Core/Src/web_server.o 

C_DEPS += \
./Core/Src/AI_Normalisation.d \
./Core/Src/AO_Normalisation.d \
./Core/Src/Block_RTC.d \
./Core/Src/Block_Sifu.d \
./Core/Src/Block_Synhro.d \
./Core/Src/DB_Constants.d \
./Core/Src/DB_Main.d \
./Core/Src/DB_Parameters.d \
./Core/Src/DI_Block.d \
./Core/Src/DO_Block.d \
./Core/Src/Reg_System.d \
./Core/Src/event_log.d \
./Core/Src/main.d \
./Core/Src/stm32f7xx_hal_msp.d \
./Core/Src/stm32f7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f7xx.d \
./Core/Src/trace_buffer.d \
./Core/Src/web_server.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Middlewares/Third_Party/LwIP/src/apps/http -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/AI_Normalisation.cyclo ./Core/Src/AI_Normalisation.d ./Core/Src/AI_Normalisation.o ./Core/Src/AI_Normalisation.su ./Core/Src/AO_Normalisation.cyclo ./Core/Src/AO_Normalisation.d ./Core/Src/AO_Normalisation.o ./Core/Src/AO_Normalisation.su ./Core/Src/Block_RTC.cyclo ./Core/Src/Block_RTC.d ./Core/Src/Block_RTC.o ./Core/Src/Block_RTC.su ./Core/Src/Block_Sifu.cyclo ./Core/Src/Block_Sifu.d ./Core/Src/Block_Sifu.o ./Core/Src/Block_Sifu.su ./Core/Src/Block_Synhro.cyclo ./Core/Src/Block_Synhro.d ./Core/Src/Block_Synhro.o ./Core/Src/Block_Synhro.su ./Core/Src/DB_Constants.cyclo ./Core/Src/DB_Constants.d ./Core/Src/DB_Constants.o ./Core/Src/DB_Constants.su ./Core/Src/DB_Main.cyclo ./Core/Src/DB_Main.d ./Core/Src/DB_Main.o ./Core/Src/DB_Main.su ./Core/Src/DB_Parameters.cyclo ./Core/Src/DB_Parameters.d ./Core/Src/DB_Parameters.o ./Core/Src/DB_Parameters.su ./Core/Src/DI_Block.cyclo ./Core/Src/DI_Block.d ./Core/Src/DI_Block.o ./Core/Src/DI_Block.su ./Core/Src/DO_Block.cyclo ./Core/Src/DO_Block.d ./Core/Src/DO_Block.o ./Core/Src/DO_Block.su ./Core/Src/Reg_System.cyclo ./Core/Src/Reg_System.d ./Core/Src/Reg_System.o ./Core/Src/Reg_System.su ./Core/Src/event_log.cyclo ./Core/Src/event_log.d ./Core/Src/event_log.o ./Core/Src/event_log.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f7xx_hal_msp.cyclo ./Core/Src/stm32f7xx_hal_msp.d ./Core/Src/stm32f7xx_hal_msp.o ./Core/Src/stm32f7xx_hal_msp.su ./Core/Src/stm32f7xx_it.cyclo ./Core/Src/stm32f7xx_it.d ./Core/Src/stm32f7xx_it.o ./Core/Src/stm32f7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f7xx.cyclo ./Core/Src/system_stm32f7xx.d ./Core/Src/system_stm32f7xx.o ./Core/Src/system_stm32f7xx.su ./Core/Src/trace_buffer.cyclo ./Core/Src/trace_buffer.d ./Core/Src/trace_buffer.o ./Core/Src/trace_buffer.su ./Core/Src/web_server.cyclo ./Core/Src/web_server.d ./Core/Src/web_server.o ./Core/Src/web_server.su

.PHONY: clean-Core-2f-Src

