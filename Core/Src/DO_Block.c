#include "main.h"
#include "DO_Block.h"
#include "DB_Parameters.h"

__attribute__((section(".dtcm_data")))
volatile DO_Data_t DOutputsGr1;
const DO_Data_t DOutputsGr1Default ={
		.do1 = 0
};

__attribute__((section(".itcm_text")))
void DO_XOR(void){
	i380V_Podano_DO = DBMain.b32.i380V_Podano_DOxor ^ DBParameters.b64.P70_1;
	i380V_Znyato_DO = DBMain.b32.i380V_Znyato_DOxor ^ DBParameters.b64.P70_2;
	Merezha380vFault_DO = DBMain.b32.Merezha380vFault_DOxor ^ DBParameters.b64.P70_3;
	GeneratorFault_DO = DBMain.b32.GeneratorFault_DOxor ^ DBParameters.b64.P70_4;
	i380V_Vvod_DO = DBMain.b32.i380V_Vvod_DOxor ^ DBParameters.b64.P70_5;
	BlockDoor_DO = DBMain.b32.BlockDoor_DOxor ^ DBParameters.b64.P70_6;
	Rezerv1_DO = DBMain.b32.Rezerv1_DOxor ^ DBParameters.b64.P70_7;
	Rezerv2_DO = DBMain.b32.Rezerv2_DOxor ^ DBParameters.b64.P70_8;
	Rezerv3_DO = DBMain.b32.Rezerv3_DOxor ^ DBParameters.b64.P70_9;
	Rezerv4_DO = DBMain.b32.Rezerv4_DOxor ^ DBParameters.b64.P70_10;
	Rezerv5_DO = DBMain.b32.Rezerv5_DOxor ^ DBParameters.b64.P70_11;
	Rezerv6_DO = DBMain.b32.Rezerv6_DOxor ^ DBParameters.b64.P70_12;
	Rezerv7_DO = DBMain.b32.Rezerv7_DOxor ^ DBParameters.b64.P70_13;
	Rezerv8_DO = DBMain.b32.Rezerv8_DOxor ^ DBParameters.b64.P70_14;

}

__attribute__((section(".itcm_text")))
void Write_DO_Output(void){
	GPIOG->ODR = (GPIOG->ODR & ~0x3FFF) | (DOutputsGr1.all & 0x3FFF);
}
