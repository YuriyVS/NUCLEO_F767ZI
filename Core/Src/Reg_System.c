#include "main.h"
#include "Reg_System.h"
#include "DB_Main.h"
#include "DB_Parameters.h"
#include "DB_Constants.h"



void Reg_System_Proces(float Tscan) {
    // --- 1. Шунтирование ---
    bool shunt = DBMain.b64.ShuntReg;
    DBMain.b96.ShuntZiN = DBMain.b96.ShuntRN = DBMain.b64.ShuntZiT1 = DBMain.b64.ShuntZiT2 = DBMain.b64.ShuntRT1 = DBMain.b64.ShuntRT2 = shunt;

    // --- 2. ЗИ напряжения (Ramp Generator) ---
    if (DBParameters.b32.P30_1 == 0) {
    	DBMain.f50.ZiNout = DBMain.f50.Ud_ref;
    } else {
        if (DBMain.b96.ShuntZiN) {
        	DBMain.f50.ZiNout = 0.0f;
        } else {
            // Выбор темпа останова
        	DBMain.f50.ZiNTiStop = DBMain.b96.EStopMem ? DBParameters.f100.P31_3 : DBParameters.f100.P31_2;

            if (DBMain.f50.Ud_ref > DBMain.f50.ZiNout) {
                if (DBMain.f50.Ud_ref > 0.0f) {
                    if (DBParameters.f100.P31_1 == 0.0f) {DBMain.f50.ZiNout = DBMain.f50.Ud_ref;}
                    else {DBMain.f50.ZiNout += 100.0f * Tscan / (DBParameters.f100.P31_1 * 1000.0f);}
                } else {
                    if (DBMain.f50.ZiNTiStop == 0.0f) {DBMain.f50.ZiNout = DBMain.f50.Ud_ref;}
                    else {DBMain.f50.ZiNout += 100.0f * Tscan / (DBMain.f50.ZiNTiStop* 1000.0f);}
                }
                if (DBMain.f50.ZiNout > DBMain.f50.Ud_ref) {DBMain.f50.ZiNout = DBMain.f50.Ud_ref;}
            }
            else if (DBMain.f50.Ud_ref < DBMain.f50.ZiNout) {
                if (DBMain.f50.Ud_ref < 0.0f) {
                    if (DBParameters.f100.P31_1 == 0.0f) {DBMain.f50.ZiNout = DBMain.f50.Ud_ref;}
                    else {DBMain.f50.ZiNout -= 100.0f * Tscan / (DBParameters.f100.P31_1 * 1000.0f);}
                } else {
                    if (DBMain.f50.ZiNTiStop == 0.0f) {DBMain.f50.ZiNout = DBMain.f50.Ud_ref;}
                    else {DBMain.f50.ZiNout -= 100.0f * Tscan / (DBMain.f50.ZiNTiStop* 1000.0f);}
                }
                if (DBMain.f50.ZiNout < DBMain.f50.Ud_ref) {DBMain.f50.ZiNout = DBMain.f50.Ud_ref;}
            }
        }
    }

    // --- 3. Регулятор напряжения ---
    DBMain.f50.RNref = DBMain.f50.ZiNout;
    if (DBParameters.b32.P30_2 == 0) {
    	DBMain.f50.RNout = DBMain.f50.RNref;
    	DBMain.f50.IntChRN = 0.0f;
 //   	DBMain.f50.RefUovg = DBParameters.f100.P34_6;
    } else if (DBMain.b96.ShuntRN) {
    	DBMain.f50.RNout = 0.0f;
    	DBMain.f50.IntChRN = 0.0f;
 //   	DBMain.f50.RefUovg = DBParameters.f100.P34_6;
    } else {
    	DBMain.f50.Ndelta = DBMain.f50.RNref - DBMain.f50.Useti;
        if (DBParameters.b64.P30_3 == 0 || DBParameters.f100.P32_2 == 0.0f) {
        	DBMain.f50.IntChRN = 0.0f;
        } else {
            // Передача управления от токовых регуляторов (Anti-windup/Handover)
//            if (!Main.Rezerv2_DI && (Main.Ndelta < 0.0f) && (Main.IntChRT1 > 0.0f || Main.IntChRT2 > 0.0f)) {
//                Main.IntChRN -= (Main.IntChRT1 + Main.IntChRT2);
//                Main.IntChRT1 = 0.0f;
//                Main.IntChRT2 = 0.0f;
//            }
        	DBMain.f50.IntChRN += DBMain.f50.Ndelta * Tscan / DBParameters.f100.P32_2;
        }

        // Ограничение интеграла
        if (DBMain.f50.IntChRN > DBParameters.f100.P32_3) {DBMain.f50.IntChRN = DBParameters.f100.P32_3;}
        else if (DBMain.f50.IntChRN < DBParameters.f100.P32_4) {DBMain.f50.IntChRN = DBParameters.f100.P32_4;}

        DBMain.f50.RNout = DBMain.f50.IntChRN + DBMain.f50.Ndelta * DBParameters.f100.P32_1;

        // Ограничение выхода
        if (DBMain.f50.RNout > DBParameters.f100.P32_3) {DBMain.f50.RNout = DBParameters.f100.P32_3;}
        else if (DBMain.f50.RNout < DBParameters.f100.P32_4) {DBMain.f50.RNout = DBParameters.f100.P32_4;}

 //       Main.RefUovg = Params.P25_5;
    }

    // --- 4. Регулятор тока нагрузки (RT1) ---
    if (DBParameters.b32.P30_4 == 0 || DBMain.b64.ShuntRT1) {
    	DBMain.f50.RTout1 = 0.0f;
    	DBMain.f50.IntChRT1 = 0.0f;
    } else {
    	DBMain.f50.Idelta1 = DBMain.f50.IgenRef - DBMain.f50.Iseti;
        if (DBParameters.b32.P30_5 != 0 && DBParameters.f100.P33_2 != 0.0f) {
        	DBMain.f50.IntChRT1 -= DBMain.f50.Idelta1 * Tscan / DBParameters.f100.P33_2;
        } else {
        	DBMain.f50.IntChRT1 = 0.0f;
        }

        if (DBMain.f50.IntChRT1 > DBParameters.f100.P33_3) {DBMain.f50.IntChRT1 = DBParameters.f100.P33_3;}
        else if (DBMain.f50.IntChRT1 < DBParameters.f100.P33_4) {DBMain.f50.IntChRT1 = DBParameters.f100.P33_4;}

        DBMain.f50.RTout1 = DBMain.f50.IntChRT1 - DBMain.f50.Idelta1 * DBParameters.f100.P33_1;

        if (DBMain.f50.RTout1 > DBParameters.f100.P33_3) {DBMain.f50.RTout1 = DBParameters.f100.P33_3;}
        else if (DBMain.f50.RTout1 < DBParameters.f100.P33_4) {DBMain.f50.RTout1 = DBParameters.f100.P33_4;}
    }

    // --- 5. Регулятор тока аккумулятора (RT2) ---
    if (DBParameters.b32.P30_6 == 0 || DBMain.b64.ShuntRT2) {
    	DBMain.f50.RTout2 = 0.0f;
    	DBMain.f50.IntChRT2 = 0.0f;
    } else {
    	DBMain.f50.Idelta2 = DBMain.f50.IakbRef - DBMain.f50.Iakb;
        if (DBParameters.b32.P30_7 != 0 && DBParameters.f100.P34_2 != 0.0f) {
        	DBMain.f50.IntChRT2 -= DBMain.f50.Idelta2 * Tscan / DBParameters.f100.P34_2;
        } else {
        	DBMain.f50.IntChRT2 = 0.0f;
        }

        if (DBMain.f50.IntChRT2 > DBParameters.f100.P34_3) {DBMain.f50.IntChRT2 = DBParameters.f100.P34_3;}
        else if (DBMain.f50.IntChRT2 < DBParameters.f100.P34_4) {DBMain.f50.IntChRT2 = DBParameters.f100.P34_4;}

        DBMain.f50.RTout2 = DBMain.f50.IntChRT2 - DBMain.f50.Idelta2 * DBParameters.f100.P34_1;

        if (DBMain.f50.RTout2 > DBParameters.f100.P34_3) {DBMain.f50.RTout2 = DBParameters.f100.P34_3;}
        else if (DBMain.f50.RTout2 < DBParameters.f100.P34_4) {DBMain.f50.RTout2 = DBParameters.f100.P34_4;}
    }

    // --- 6. Выход системы (Сумматор) ---
    float refIovg_local = DBMain.f50.RNout - DBMain.f50.RTout1 - DBMain.f50.RTout2;

    if (refIovg_local > DBParameters.f100.P34_7) {DBMain.f50.RefIovg = DBParameters.f100.P34_7;}
    else if (refIovg_local < DBParameters.f100.P34_8) {DBMain.f50.RefIovg = DBParameters.f100.P34_8;}
    else DBMain.f50.RefIovg = refIovg_local;
}
