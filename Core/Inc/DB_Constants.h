#ifndef DB_CONSTANTS_H
#define DB_CONSTANTS_H

#include <stdint.h>
#include <stdbool.h>

// Константы Float
typedef union {
    struct {
        float TaktFilterAO;
        float TaktBlockGot;
        float TaktTehnolAlg;
        float TaktSystReg;
        float ItintMax;
        float IntChRT2max;
        float IntChRT2min;
        float IntChRT1max;
        float IntChRT1min;
        float IntChRNmax;
        float IntChRNmin;
        float fconst12;
        float fconst13;
        float fconst14;
        float fconst15;
        float fconst16;
        float fconst17;
        float fconst18;
        float fconst19;
        float fconst20;
        float fconst21;
        float fconst22;
        float fconst23;
        float fconst24;
        float fconst25;
        float fconst26;
        float fconst27;
        float fconst28;
        float fconst29;
        float fconst30;
        float fconst31;
        float fconst32;
        float fconst33;
        float fconst34;
        float fconst35;
        float fconst36;
        float fconst37;
        float fconst38;
        float fconst39;
        float fconst40;
        float fconst41;
        float fconst42;
        float fconst43;
        float fconst44;
        float fconst45;
        float fconst46;
        float fconst47;
        float fconst48;
        float fconst49;
        float fconst50;

    };
    float as_array[50];
} DB_Constants_Float_ID1_50;

// Константы Int
typedef union {
    struct {
        int32_t iconst1;
        int32_t iconst2;
        int32_t iconst3;
        int32_t iconst4;
        int32_t iconst5;
        int32_t iconst6;
        int32_t iconst7;
        int32_t iconst8;
        int32_t iconst9;
        int32_t iconst10;
        int32_t iconst11;
        int32_t iconst12;
        int32_t iconst13;
        int32_t iconst14;
        int32_t iconst15;
        int32_t iconst16;
        int32_t iconst17;
        int32_t iconst18;
        int32_t iconst19;
        int32_t iconst20;
        int32_t iconst21;
        int32_t iconst22;
        int32_t iconst23;
        int32_t iconst24;
        int32_t iconst25;
        int32_t iconst26;
        int32_t iconst27;
        int32_t iconst28;
        int32_t iconst29;
        int32_t iconst30;
        int32_t iconst31;
        int32_t iconst32;
        int32_t iconst33;
        int32_t iconst34;
        int32_t iconst35;
        int32_t iconst36;
        int32_t iconst37;
        int32_t iconst38;
        int32_t iconst39;
        int32_t iconst40;
        int32_t iconst41;
        int32_t iconst42;
        int32_t iconst43;
        int32_t iconst44;
        int32_t iconst45;
        int32_t iconst46;
        int32_t iconst47;
        int32_t iconst48;
        int32_t iconst49;
        int32_t iconst50;
    };
    int32_t as_array[50];
} DB_Constants_Int_ID1_50;

// Константы UInt
typedef union {
    struct {
        int32_t uconst1;
        int32_t uconst2;
        int32_t uconst3;
        int32_t uconst4;
        int32_t uconst5;
        int32_t uconst6;
        int32_t uconst7;
        int32_t uconst8;
        int32_t uconst9;
        int32_t uconst10;
        int32_t uconst11;
        int32_t uconst12;
        int32_t uconst13;
        int32_t uconst14;
        int32_t uconst15;
        int32_t uconst16;
        int32_t uconst17;
        int32_t uconst18;
        int32_t uconst19;
        int32_t uconst20;
        int32_t uconst21;
        int32_t uconst22;
        int32_t uconst23;
        int32_t uconst24;
        int32_t uconst25;
        int32_t uconst26;
        int32_t uconst27;
        int32_t uconst28;
        int32_t uconst29;
        int32_t uconst30;
        int32_t uconst31;
        int32_t uconst32;
        int32_t uconst33;
        int32_t uconst34;
        int32_t uconst35;
        int32_t uconst36;
        int32_t uconst37;
        int32_t uconst38;
        int32_t uconst39;
        int32_t uconst40;
        int32_t uconst41;
        int32_t uconst42;
        int32_t uconst43;
        int32_t uconst44;
        int32_t uconst45;
        int32_t uconst46;
        int32_t uconst47;
        int32_t uconst48;
        int32_t uconst49;
        int32_t uconst50;
    };
    uint32_t as_array[50];
} DB_Constants_UInt_ID1_50;

// Константы Bool (конфигурационные флаги сборки)
typedef union {
    const uint32_t all;
    struct {
    	bool bit0 : 1;  bool bit1 : 1;  bool bit2 : 1;  bool bit3 : 1;
    	bool bit4 : 1;  bool bit5 : 1;  bool bit6 : 1;  bool bit7 : 1;
    	bool bit8 : 1;  bool bit9 : 1;  bool bit10: 1;  bool bit11: 1;
    	bool bit12: 1;  bool bit13: 1;  bool bit14: 1;  bool bit15: 1;
    	bool bit16: 1;  bool bit17: 1;  bool bit18: 1;  bool bit19: 1;
    	bool bit20: 1;  bool bit21: 1;  bool bit22: 1;  bool bit23: 1;
    	bool bit24: 1;  bool bit25: 1;  bool bit26: 1;  bool bit27: 1;
    	bool bit28: 1;  bool bit29: 1;  bool bit30: 1;  bool bit31: 1;
    };
} DB_Constants_Bool_ID1_32;
typedef union {
    const uint32_t all;
    struct {
    	bool bit0 : 1;  bool bit1 : 1;  bool bit2 : 1;  bool bit3 : 1;
    	bool bit4 : 1;  bool bit5 : 1;  bool bit6 : 1;  bool bit7 : 1;
    	bool bit8 : 1;  bool bit9 : 1;  bool bit10: 1;  bool bit11: 1;
    	bool bit12: 1;  bool bit13: 1;  bool bit14: 1;  bool bit15: 1;
    	bool bit16: 1;  bool bit17: 1;  bool bit18: 1;  bool bit19: 1;
    	bool bit20: 1;  bool bit21: 1;  bool bit22: 1;  bool bit23: 1;
    	bool bit24: 1;  bool bit25: 1;  bool bit26: 1;  bool bit27: 1;
    	bool bit28: 1;  bool bit29: 1;  bool bit30: 1;  bool bit31: 1;
    };
} DB_Constants_Bool_ID33_64;

// Главный контейнер констант
typedef struct {
	DB_Constants_Float_ID1_50 f50;
	DB_Constants_Int_ID1_50   i50;
	DB_Constants_UInt_ID1_50  u50;
	DB_Constants_Bool_ID1_32  b32;
	DB_Constants_Bool_ID33_64 b64;
} DB_Constants;

extern const DB_Constants DBConstants;

#endif
