/*компил€ци€ использу€ -lncurses
 например, gcc absgraph.c -o absgraph -lncurses
 */
#include <stdio.h>                                /*подкл.библ.ф-й ст.в/выв */
#include <string.h>                               /*подкл.библ.ф-й стр.симв.*/
#include <stdlib.h>                               /*подкл.библ.ф-й преобр.д.*/
#include <ctype.h>                                /*подкл.библ.ф-й преобр.с.*/
#include <curses.h>

#define  NSPIS  5                                 /*разм.списка загр.прогр. */
#define  NOBJ   50                                /*разм.масс.об'ектных карт*/
#define  DOBLZ  1024                              /*длина области загрузки  */
#define  NOP 16                                    /*кол-во обрабатываемых   */
/* команд                 */
int reg3_count = 0;
int RES = -1;
int RES_R = -1;
char NFIL [30] = "\x0";
int massTemp[9] = {1, 0, 1, 20, 20, 1, 20, 20, 20};

int  IOBJC   = 0;                                 /*инд.вакантн.стр. OBJCARD*/
char OBJCARD [NOBJ][100]; //80                         /*масс.хранен.об'ектн.карт*/
long onechar = 0x31L;
long zero = 0x30L;
int counter = 9;

int  ISPIS   = 0;                                 /*инд.вакантн.стр. SPISOK */
char SPISOK  [NSPIS][100];     //80                    /*масс.хранен.списка прогр*/

WINDOW *wblue, *wgreen, *wred, *wcyan, *wmargenta;

struct STR_BUF_TXT                                /*структ.буфера карты TXT */
{
    unsigned char POLE1      ;                      /*место дл€ кода 0x02     */
    unsigned char POLE2  [ 3];                      /*поле типа об'ектн.карты */
    unsigned char POLE3      ;                      /*пробел                  */
    unsigned char ADOP   [ 3];                      /*относит.адрес опреации  */
    unsigned char POLE5  [ 2];                      /*пробелы                 */
    unsigned char DLNOP  [ 2];                      /*длина операции          */
    unsigned char POLE7  [ 2];                      /*пробелы                 */
    unsigned char POLE71 [ 2];                      /*внутренний идент.прогр. */
    unsigned char OPER   [56];                      /*тело операции           */
    unsigned char POLE9  [ 8];                      /*идентификационное поле  */
};


union                                             /*определить об'единение  */
{
    struct STR_BUF_TXT STR_TXT;                     /*структура буфера        */
    unsigned char BUF_TXT [100];  //80                   /*буфер карты TXT         */
} TXT;


unsigned char INST [6];                           /*массив, содерж. обрабат.*/
/*команду                 */


/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int FRR();                                        /*подпр.обр.опер.RR-форм. */

/*..........................................................................*/

/*п р о т о т и п  обращ.к*/
int FRX();                                        /*подпр.обр.опер.RX-форм. */


int FSS();
/*..........................................................................*/


int X1 = 1;                                       /* инициализаци€ коорд.   */
int Y1 = 15;                                      /* на экране              */
int R1,                                           /*номер 1-го регистра-опе-*/
/*ранда в форматах RR и RX*/
R2,                                           /*номер 2-го регистра-опе-*/
/*ранда в формате RX      */
D,                                            /*смещение в формате RX   */
X,                                            /*номер индексн. регистра */
/*в формате RX            */
B;                                            /*номер базового регистра */
/*в формате RX            */
int L, B1, D1, B2, D2;
unsigned long I,                                  /*счетчик адр.тек.ком-ды  */
BAS_ADDR,                           /*адрес начала обл.загруз.*/
I1,ADDR,ARG,VS;                     /*вспомогательные перем.  */
unsigned long VR[16],                             /*массив,содерж.знач.рег. */
LIGHTPTR;                           /*адрес начала обл.отсвет.*/


int x,y,i,j,k,kk;                                 /*рабочие переменные      */

int CUR_IND;                                      /*индекс масс.обл.загр.,  */
/*соотв.текущ.ком-де прогр*/
int BAS_IND;                                      /*индекс масс.обл.загр.,  */
/*соотв.первой ком-ды прог*/

union U1                                        /*посто€нна€ часть шабло- */
{                                              /*на отсветки регистров на*/
    struct                                        /*экране консоли          */
    {
        char NOMREG  [ 3];
        char ZNEQUEL [1];
    } REGS_ASC [16];
    
    char BUFR [16][4];
}R_ASC;

union u2                                        /*шаблон дл€ расчета      */
{                                              /*элементов абсолютного   */
    struct
    {
        unsigned int SMESH;
        unsigned int SEGM;
    } VAL_P;
    unsigned char *P_OBLZ ;
} POINT;

unsigned char OBLZ [DOBLZ] ;                    /*область загрузки трас-  */
/*сируемой программы      */
/*
 ***** “јЅЋ»÷ј машинных операций
 */

struct TMOP                                      /*структ.стр.табл.маш.опер*/
{
    unsigned char MNCOP [5];                       /*мнемокод операции       */
    unsigned char CODOP    ;                       /*машинный код операции   */
    unsigned char DLOP     ;                       /*длина операции в байтах */
    int (*BXPROG)()        ;                       /*указатель на подпр.обраб*/
} T_MOP [NOP]  =                                /*об'€вление табл.маш.опер*/
{
    {{'B' , 'A' , 'L' , 'R' , ' '} , '\x05', 2 , FRR},/*инициализаци€           */
    {{'B' , 'C' , 'R' , ' ' , ' '} , '\x07', 2 , FRR}, /*строк                   */
    {{'S' , 'T' , ' ' , ' ' , ' '} , '\x50', 4 , FRX}, /*таблицы                 */
    {{'L' , ' ' , ' ' , ' ' , ' '} , '\x58', 4 , FRX}, /*машинных                */
    {{'A' , ' ' , ' ' , ' ' , ' '} , '\x5A', 4 , FRX}, /*операций                */
    {{'S' , ' ' , ' ' , ' ' , ' '} , '\x5B', 4 , FRX}, /*
                                                        */
    {{'M','V','C',' ',' '} , '\xD2' , 6 , FSS} ,
    {{'L','A',' ',' ',' '} , '\x41' , 4 , FRX} ,
    {{'C',' ',' ',' ',' '} , '\x59' , 4 , FRX} ,
    {{'B','C',' ',' ',' '} , '\x47' , 4 , FRX} ,
    {{'I','C',' ',' ',' '} , '\x43' , 4 , FRX} ,
    {{'C','R',' ',' ',' '} , '\x19' , 2 , FRR} ,
    {{'S','R','L',' ',' '} , '\x88' , 6 , FRX} ,
    {{'O','R',' ',' ',' '} , '\x16' , 2 , FRR} ,
    {{'S','T','C',' ',' '} , '\x42' , 4 , FRX} ,
    //{{'S','T','C',' ',' '} , '\x18' , 4 , FRX} ,
};
//..........................................................................

int P_MVC(void) {
    //PRINT("MVC - change addr - %0lX %d\n", ADDR, L);
    //PRINT("B1 %d, D1 %d, B2 %d, D2 %d, L %d\n", B1, D1, B2, D2, L);
    
    int sm1, sm2, i;
    
    ADDR = VR[B1] + D1;
    sm1 = (int) ( ADDR - I );
    ADDR = VR[B2] + D2;
    sm2 = (int) ( ADDR - I );
    
    for (i=0; i<L; i++) {
        OBLZ[BAS_IND + CUR_IND + sm1 + i] = OBLZ[BAS_IND + CUR_IND + sm2 + i];
    }
    
    ADDR = VR[B] + VR[X] + D;
    
    return 0;
    
}
int P_LA(void) {
    int sm;
    /* Вычисление абсолютного адреса операнда */
    ADDR = VR[B] + VR[X] + D;
    /* Вычисление смещения операнда */
    sm = (int)(ADDR - I);
    /* Загрузка операнда на регистр */
    VR[R1] = OBLZ[BAS_IND + CUR_IND + sm] * 0x100L + OBLZ[BAS_IND + CUR_IND + sm + 1];
    
    return 0;
}
int P_C(void) {
    if (VR[R1] == counter)
    {
        RES = 0;
    }
    else if (VR[R1] < counter)
    {
        RES = 1;
    }
    else
    {
        RES = 2;
    }
    
    return 0;
}
int P_BC(void) {
    /* Проверка признака результата */
    if (RES_R == 0 || RES == 0) {
        R1 = 15;
        RES_R = -1;
    }
    if (R1 == 15 && ((R1 & 8) && RES == 0 || (R1 & 4) && RES == 1 || (R1 & 2) && RES == 2))
    {
        /* Установка адреса перехода */
        ADDR = VR[B] + VR[X] + D;
        CUR_IND+= ADDR-I;
        I = ADDR;
    }
    //    else if (RES_R == 0)
    //    {
    //        ADDR = VR[B] + VR[X] + D;
    //        CUR_IND+= ADDR-I;
    //        I = ADDR;
    //    }
    return 0;
    
}
int P_IC(void) {
    int sm;
    
    ADDR = VR[B] + VR[X] + D;
    sm = (int) ( ADDR - I );
flag2:    if (massTemp[VR[6]] == 1) {
    VR[4] = onechar;
} else if (massTemp[VR[6]] == 0) {
    VR[4] = zero;
} else if (massTemp[VR[6]] == 20) {
    VR[4] = 0x20L;
    //        VR[6] += 1;
    //        goto flag2;
}
    
    return 0;
}
int P_CR(void) {
    if (VR[4] == onechar)
    {
        RES_R = 0;
    }
    else if (VR[4] < onechar)
    {
        RES_R = 1;
    }
    else
    {
        RES_R = 2;
    }
    
    return 0;
}
int P_SRL(void) {
    if (massTemp[VR[6]] != 20) {
        VR[3] = VR[3] >> 1;
    }
    if (VR[6] == 9) {
        VR[2] = VR[2] >> 28;
    }
    return 0;
}
int P_OR(void) {
    VR[R1] = VR[R1] | VR[R2];
    return 0;
}
int P_STC(void) {
    int sm,i;
    char bytes[2];
    
    /* Вычисление абсолютного адреса операнда */
    ADDR = VR[B] + VR[X] + D;
    /* Вычисление смещения операнда */
    sm = (int)(ADDR - I);
    
    /* Преобразование числа для записи */
    bytes[0] = (VR[R1] - VR[R1]% 0x100L)/0x100L;
    bytes[1] = VR[R1]%0x100;
    
    /* Запись данных в память */
    for (i=0; i<2; i++)
        OBLZ[BAS_IND + CUR_IND + sm + i] = bytes[i];
    
    return 0;

}

//п р о г р а м м а реализации семантики команды BALR
int P_BALR(void)
    {
        if (R2 != 0)
            I = VR[R2];
        if (R1 != 0)
            VR[R1] = I;
        
        return 0;
    }
                //..........................................................................
                //п р о г р а м м а реализации семантики команды BCR с маской 15
int P_BCR(void)
    {
        int ret;
        
        ret = 1;
        if (R1 == 15)
        {
            ret = 0;
            if ((VR[R2] != 0) && (R2 != 0))
                I = VR[R2];
            else
            {
                if (R2 != 0)
                {
                    waddstr(wcyan, "переход по адресу = 0 или завершение трассировки программы после нажати€ клавиши");
                    wrefresh(wcyan);
                    ret = 1;
                }
            }
        }
        
        return ret;
    }
    /*..........................................................................*/
                
                int P_ST()                                        /*  п р о г р а м м а     */
                /*реализации семантики    */
                {                                                /*команды ST              */
                    int sm,i;                                       /*рабочие                 */
                    char bytes[4];                                  /*переменные              */
                    
                    ADDR = VR[B] + VR[X] + D;                       /*вычисление абс.адреса и */
                    sm = (int) (ADDR -I);                           /*смещени€                */
                    
                    bytes[0] = (VR[R1] -                            /*преобразование содержим.*/
                                VR[R1]% 0x1000000L)/0x1000000L;   /*–ќЌ, использованного в  */
                    bytes[1] = ((VR[R1] -                           /*качестве первого оп-да, */
                                 VR[R1]%0x10000L)/0x10000L)%0x100; /*к виду, прин€тому в     */
                    bytes[2] = ((VR[R1] % 0x10000L) -               /*≈— Ё¬ћ                  */
                                ((VR[R1]%0x10000L)%0x100))/0x100; /*                        */
                    bytes[3] = (VR[R1] % 0x10000L) % 0x100;         /*                        */
                    
                    for (i=0; i<4; i++)                             /*запись преобразованого  */
                        OBLZ[BAS_IND + CUR_IND + sm + i] = bytes[i];   /*значени€ по адресу 2-г  */
                    /*операнда                */
                    return 0;                                       /*успешное заверш.прогр.  */
                }
                
    /*..........................................................................*/
                
                int P_L()                                         /*  п р о г р а м м а     */
                /*реализации семантики    */
                {                                                /*команды L               */
                    reg3_count++;
                    int sm;                                        /*рабоча€ переменна€      */
                    
                    ADDR = VR[B] + VR[X] + D;                      /*вычисление абс.адреса и */
                    sm = (int) ( ADDR - I );                       /*смещени€                */
                    if (reg3_count == 2) {
                        VR[R1] = 0x80000000L;
                        
                    } else {
                        VR[R1] =                                       /*преобразование содержим.*/
                        OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L +   /*второго операнда к виду,*/
                        OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L + /*прин€тому в IBM PC, и   */
                        OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +    /*запись в –ќЌ, использ.в */
                        OBLZ[BAS_IND + CUR_IND + sm + 3];             /*качестве 1-го операнда  */
                    }
                    
                    return 0;                                      /*успешное заверш.прогр.  */
                }
                
    /*..........................................................................*/
                
                int P_A()                                         /*  п р о г р а м м а     */
                /*реализации семантики    */
                {                                                /*команды A               */
                    //  int sm;                                         /*рабоча€ переменна€      */
                    //
                    //  ADDR = VR[B] + VR[X] + D;                       /*вычисление абс.адреса и */
                    //  sm = ( int ) ( ADDR -I );                       /*смещени€                */
                    //  ARG = OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L+/*формирование содержимого*/
                    //   OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L +  /*второго операнда в сог- */
                    //   OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +     /*лашени€х ≈— Ё¬ћ         */
                    //   OBLZ[BAS_IND + CUR_IND + sm + 3];              /*                        */
                    //                          /*и                       */
                    //  VR[R1] = VR[R1] + ARG;                          /*сложение с 1-м операндом*/
                    VR[R1] += 1;
                    return 0;                                       /*успешное заверш. прогр. */
                }
                
    /*..........................................................................*/
                
                int P_S()                                         /* п р о г р а м м а      */
                /* реализации семантики   */
                {                                                /* команды S              */
                    int sm;                                         /*рабоча€ переменна€      */
                    
                    ADDR = VR[B] + VR[X] + D;                       /*вычисление рабочего     */
                    sm = ( int ) ( ADDR - I );                      /*адреса и смещени€       */
                    
                    ARG = OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L+/*формирование содержимого*/
                    OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L +/*второго операнда в сог- */
                    OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +   /*лашени€х ≈— Ё¬ћ         */
                    OBLZ[BAS_IND + CUR_IND + sm + 3];            /*                        */
                    /* и                      */
                    VR[R1] = VR[R1] - ARG;                          /*выч-ие из 1-го операнда */
                    
                    return 0;                                       /*успешное заверш.прогр.  */
                }
                
                
                //..........................................................................
                int FRR(void)
    {
        int i, j;
        
        for (i = 0; i < NOP; i++)
        {
            if (INST[0] == T_MOP[i].CODOP)
            {
                waddstr(wgreen, "      ");
                for (j = 0; j < 5; j++)
                    waddch(wgreen, T_MOP[i].MNCOP[j]);
                waddstr(wgreen, " ");
                
                j = (INST[1] - (INST[1] % 0x10)) / 0x10;
                R1 = j;
                wprintw(wgreen, "%1d, ", j);
                j = INST[1] % 0x10;
                R2 = j;
                wprintw(wgreen, "%1d\n", j);
                break;
            }
        }
        
        return 0;
    }
                //...........................................................................
                int FRX(void)
    {
        int i, j;
        
        for (i = 0; i < NOP; i++)
        {
            if (INST[0] == T_MOP[i].CODOP)
            {
                waddstr(wgreen, "  ");
                for (j = 0; j < 5; j++)
                    waddch(wgreen, T_MOP[i].MNCOP[j]);
                waddstr(wgreen, " ");
                
                j = INST[1] >> 4;
                R1 = j;
                if (i == 12) {
                    R1 = 3;
                    R2 = 1;
                    j = R1;
                }
                wprintw(wgreen, "%.1d, ", j);
                
                j = INST[2] % 16;
                j = j * 256 + INST[3];
                D = j;
                wprintw(wgreen, "X'%.3X'(", j);
                
                j = INST[1] % 16;
                X = j;
                wprintw(wgreen, "%1d, ", j);
                
                j = INST[2] >> 4;
                B = j;
                wprintw(wgreen, "%1d)", j);
                
                ADDR = VR[B] + VR[X] + D;
                wprintw(wgreen,"        %.06lX       \n", ADDR);
                //if (ADDR % 4 != 0)
                //    return (0);
                //return (7);//commented
                break;
            }
        }
        
        return 0;
    }
                
                int FSS(void)
    {
        int i, j;
        
        //PRINT("FSS - change addr - %0lX\n", ADDR);
        
        for (i = 0; i < NOP; i++)
        {
            if (INST[0] == T_MOP[i].CODOP)
            {
                waddstr(wgreen, "  ");
                for (j = 0; j < 5; j++)
                    waddch(wgreen, T_MOP[i].MNCOP[j]);
                waddstr(wgreen, " ");
                
                // Вычисляем L
                j = INST[1];
                L = j + 1;
                
                // Вычисляем B1 (первые 4 бита)
                j = INST[2] >> 4;
                B1 = j;
                
                // Вычисляем D1 (вторые 12 бит)
                j = INST[2] % 16;
                j = j * 256 + INST[3];
                D1 = j;
                
                // Вывод первого операнда
                wprintw(wgreen, "X'%.3X'(%.1d,%1d), ", D1, L, B1);
                
                // Вычисляем B2 (первые 4 бита)
                j = INST[4] >> 4;
                B2 = j;
                
                // Вычисляем D2 (вторые 12 бит)
                j = INST[4] % 16;
                j = j * 256 + INST[5];
                D2 = j;
                
                // Вывод второго операнда
                wprintw(wgreen, "X'%.3X'(%1d)", D2, B2);
                wprintw(wgreen, "              ");
                
                if (ADDR % 4 != 0)
                    return (7);
                break;
            }
        }
        
        return 0;
    }
                //...........................................................................
                //---------------------------------------------------------------------------
                int wind(void)
    {
        int j1, k, temp;
        
        x = 0;
        y = 16;
        
        kk = CUR_IND;
        LIGHTPTR = I1;
        
        for (j1 = 0; j1 < 8; j1++)
        {
            wprintw(wred, "%.06lX: ", I1);
            for (j = 0; j < 4; j++)
            {
                for (k = 0; k < 4; k++)
                    wprintw(wred, "%.02X", OBLZ[BAS_IND + kk + j * 4 + k]);
                waddstr(wred, " ");
            }
            
            waddstr(wred, "/* ");
            for (j = 0; j < 16; j++)
            {
                if (isprint (OBLZ[BAS_IND + kk]) )
                {
                    waddch(wred, OBLZ[BAS_IND + kk++]);
                    wrefresh(wred);
                }
                else
                {
                    waddstr(wred, ".");
                    kk++;
                }
            }
            
            waddstr(wred, " */");
            I1 += 16;
        }
        wrefresh(wred);            //вывод на экран
        wclear(wred);                //очистка содержимого окна дампа
        
        return 0;
    }
                //---------------------------------------------------------------------------
                //программа покомандной интерпретпции(отладки)
                // загруженной программы
                int sys(void)
    {
        int res, temp;
        int ch;
        int gr_pos_x, gr_pos_y;
        int ii = 0, jj = 0;
        int gr_y;
        char wstr[100];//80
        int zizi = 0, tempI;
        
        
        I = BAS_ADDR;            //установить текущий адрес
        //равный начальному
        //нижнее поле
        wmargenta = newwin(1, 100, 24, 0); //80
        wbkgd(wmargenta, COLOR_PAIR(COLOR_MAGENTA));
        waddstr(wmargenta, "\"PgUp\",\"PgDn\",\"Up\",\"Down\"->просмотр дампа; \"Enter\"->выполнить очередную команду");
        
        //строка состо€ни€
        wcyan = newwin(1, 100, 23, 0); //80
        wbkgd(wcyan, COLOR_PAIR(COLOR_CYAN));
        
        //дамп области загрузки
        wred = newwin(8, 67, 15, 0);
        wbkgd(wred, COLOR_PAIR(COLOR_RED));
        
        //поле регистров
        wblue = newwin(16, 12, 0, 68);
        wbkgd(wblue, COLOR_PAIR(COLOR_BLUE));
        
        //текст
        gr_pos_x = 0;
        gr_pos_y = 14;
        gr_y = 11;
        wgreen = newwin(gr_y, 67, gr_pos_y, gr_pos_x);    //создадим новое окно
        wbkgd(wgreen, COLOR_PAIR(COLOR_GREEN));    //выбор цветовой пары
        
        
        keypad(wmargenta, TRUE);                //разрешить преобразование кодов клавиатуры
        
    BEGIN:
        
        //все допустимые коды к-нд сравнить с текущей и при
        //совпадениизапомнить номер строки таблицы операций
        for (i = 0; i < NOP; i++)
        {
            //      if (OBLZ[BAS_IND + CUR_IND] == '\x18') {
            //          CUR_IND = CUR_IND - 4;
            //      }
            //      unsigned char temp = '\xf0';
            //      if (OBLZ[BAS_IND + CUR_IND] == temp) {
            //          CUR_IND = CUR_IND + 6;
            //      }
            if (OBLZ[BAS_IND + CUR_IND] == T_MOP[i].CODOP)
            {
                k = i;
                wprintw(wgreen, "%.06lX: ", I);
                //рисуем окно, выводим текст
                for (j = 0; j < 6; j++)                     /*                        */
                {                                        /*                        */
                    if (i == 12 && j > 3) {
                        goto flag;
                    }
                    if (j < T_MOP[i].DLOP)                  /*                        */
                    {                                      /* выдать шестнадцатеричн.*/
                        wprintw(wgreen, "%.02X", OBLZ[BAS_IND + CUR_IND + j]);
                        /* запомнить его же в     */
                        INST[j] =                                   /* переменной INST,       */
                        OBLZ [BAS_IND + CUR_IND + j];/*                        */
                    }                                      /*                        */
                    else {
                    flag:     INST [j] = '\x00';                     /*                        */
                    }
                }
                if ((res = T_MOP[i].BXPROG()) != 0)    /* уйти в программу отобр.*/
                    return (res);                 /* ассемблерного эквивале-*/
                /* нта текущей команды,   */
                /*                        */
                goto l0;                                  /* перейти к дальнейшей  */
            }
        }
        return (6);
        
    l0:
        //сдвиг окна вверх
        wrefresh(wgreen);
        ii++;
        if (gr_pos_y > 14 - gr_y + 1)
            mvwin(wgreen, gr_pos_y--, gr_pos_x);
        //при достижении некоторого положени€, движение останавливаетс€, и производитс€
        //прокрутка окна
        else
        {
            for (jj = 0; jj < gr_y - 1; jj++)
            {
                temp = mvwinnstr(wgreen, jj + 1, 0, wstr, 67);
                mvwaddnstr(wgreen, jj, 0, wstr, 67);
                wrefresh(wgreen);
            }
        }
        wrefresh(wgreen);
        
        I += T_MOP[k].DLOP;                            /*коррекци€ счет-ка.адреса*/
        CUR_IND = ( int ) ( I - BAS_ADDR );            /*уст-ка текущ. индекса   */
        /*в массиве OBLZ          */
        I1 = I;                                        /*установка адреса начала */
        /*области отсветки        */
        
        for ( i = 0; i < 16; i++)
        {
            if (i < 10)
                waddstr(wblue, "R0");
            else
                waddstr(wblue, "R");
            wprintw(wblue, "%d:", i);
            wprintw(wblue, "%.08lX", VR[i]);
        }
        wrefresh(wblue);            //вывод на экран
        wclear(wblue);            //очистка окна регистров
        wind();
        
        waddstr(wcyan, "готовность к выполнению очередной команды с адресом ");
        wprintw(wcyan, "%.06lX", I - T_MOP[k].DLOP);
        waddstr(wcyan, "\n");
        wrefresh(wcyan);
        wclear(wcyan);
        
    WAIT:
        
        CUR_IND = (int)(I - BAS_ADDR);
        
        ch = wgetch(wmargenta);
        
        switch (ch)
        {
            case 10:
            {
                goto SKIP;
            }
                
            case  KEY_UP:
            {
                I1 = LIGHTPTR - 16;
                CUR_IND = (int)(I1 - BAS_ADDR);
                wind();
                goto WAIT;
            }
                
            case  KEY_DOWN:
            {
                I1 = LIGHTPTR + 16;
                CUR_IND = (int)(I1 - BAS_ADDR);
                wind();
                goto WAIT;
            }
                
            case  KEY_PPAGE:
            {
                I1 = LIGHTPTR - 128;
                CUR_IND = (int)(I1 - BAS_ADDR);
                wind();
                goto WAIT;
            }
                
            case  KEY_NPAGE:
            {
                I1 = LIGHTPTR + 128 ;
                CUR_IND = (int)(I1 - BAS_ADDR);
                wind();
                goto WAIT;
            }
        }
        goto WAIT;
        
    SKIP:
        
        switch (T_MOP[k].CODOP)                        //согласно  коду команды,
        {                                              //селектируемой сч.адреса
                //выбрать подпрогр.интер-
            case '\x05' : P_BALR();                       //претации семантики
                break;                         //текущей команды
            case '\x07' : { i = P_BCR();
                getch();
                if (i == 1)
                    return 8;
            }
                break;
            case '\x50' : P_ST();
                break;
            case '\x58' : P_L();
                break;
            case '\x5A' : P_A();
                break;
            case '\x5B' : P_S();
                break;
            case '\xD2' : P_MVC();
                break;
            case '\x41' : P_LA();
                break;
            case '\x59' : P_C();
                break;
            case '\x47' : P_BC();
                break;
            case '\x43' : P_IC();
                break;
            case '\x19' : P_CR();
                break;
            case '\x88' : P_SRL();
                break;
            case '\x16' : P_OR();
                break;
            case '\x42' : P_STC();
                break;
                //       case '\x18' : P_STC();
                //           break;
        }
        unsigned char char1 = '\xD2';
        unsigned char char2 = '\x88';
        if (T_MOP[k].CODOP == char1) {
            P_MVC();
        }
        if (T_MOP[k].CODOP == char2) {
            P_SRL();
        }
        
        goto BEGIN;
        
        delwin(wblue);
        delwin(wred);
        delwin(wgreen);
        delwin(wmargenta);
        
        return 0;
    }
                //...........................................................................
                //..........................»нициализаци€ curses..............................
                int InitCurses(void)
    {
        initscr();                    //инициализаци€ библиотеки curses
        curs_set(0);
        noecho();                    //не показывать ввод
        cbreak();                    //читать один символ
        //за раз, не ждать \n
        keypad(stdscr, TRUE);                //разрешить преобразование кодов клавиатуры
        start_color();
        
        
        if (has_colors())
        {
            init_pair(COLOR_BLUE, COLOR_WHITE, COLOR_BLUE);
            init_pair(COLOR_GREEN, COLOR_BLACK, COLOR_GREEN);
            init_pair(COLOR_RED, COLOR_WHITE, COLOR_RED);
            init_pair(COLOR_CYAN, COLOR_BLACK, COLOR_CYAN);
            init_pair(COLOR_MAGENTA, COLOR_WHITE, COLOR_MAGENTA);
        }
        
        
        return 0;
    }
                //...........................................................................
                
                int main( int argc, char **argv )                /* п р о г р а м м а      */
                /*абсолютного загрузчика  */
                /*об'ектных файлов        */
    {
        int  I,K,N,J0,res;                              /*рабочие                 */
        unsigned long J;                                /*переменные              */
        FILE *fp;                                       /*программы               */
        char *ptr;
        
        //main programm
        
        
        if ( argc != 2 )
        {
            printf ( "%s\n", "ќшибка в командной строке" );
            return -1;
        }
        
        ptr = argv[1];
        strcpy ( NFIL, ptr );
        
        if ( strcmp ( &NFIL [ strlen ( NFIL )-3 ], "mod" ) )
        {
            goto ERR9;
            return -1;
        }
        
        if ((fp = fopen(NFIL,"rt")) == NULL)
            goto ERR1;                                     /*сообщение об ошибке     */
        else
        {
            while ( !feof( fp ) )                         /*читать все карты файла  */
            {                                            /*со списком              */
                fgets ( SPISOK [ISPIS++] , 100 , fp );       /*в массив SPISOK         */ //80
                if ( ISPIS == NSPIS )                       /*если этот массив пере-  */
                {                                          /*полнен, то:             */
                    fclose ( fp );                            /*закрыть файл со списком */
                    goto ERR4;                                /*и выдать сообщение об ош*/
                }
            }
            fclose ( fp );                                /*закрыть файл SPISOK     */
            
            if ( ISPIS == 0 )                             /*если список пустойб     */
            /*то:                     */
                goto ERR2;                                   /* сообщение об ошибке    */
            else                                          /*иначе:                  */
                goto CONT1;                                  /* продолжить обработку   */
        }
        
    CONT1:
        
        for ( I = 0; I < ISPIS; I++ )                   /*перебира€ все собираемые*/
        {
            /*об'ектные файлы,        */
            if ((fp = fopen(SPISOK[I], "rb" )) ==  NULL)
                goto ERR3;                                   /*                        */
            else                                          /* иначе:                 */
            {                                          /*                        */
                while ( !feof( fp) )                        /*  читать файл до конца, */
                {                                          /*  размеcтить записи в   */
                    fread ( OBJCARD [IOBJC++] , 80 , 1 , fp );/*  массиве OBJCARD и,если*/ //80
                    if ( IOBJC == NOBJ )                      /*  считаны не все записи,*/
                    {                                        /*  то:                   */
                        fclose ( fp );                          /*   выдать сообщ.об ошиб.*/
                        goto ERR5;                              /*                        */
                    }
                }                                          /*                        */
                fclose ( fp );                              /*  закрыть очередной файл*/
                
                goto CONT2;                                 /*  и продолжить обработку*/
            }
        }
        
    CONT2:
        
        POINT.P_OBLZ = OBLZ;                            /*расчитать абсолютный    */
        J = POINT.VAL_P.SEGM ;                          /*адрес области загрузки  */
        J = J << 4;                                     /*OBLZ в переменной J     */
        J += POINT.VAL_P.SMESH;
        
        if ( ( J0 = (int) J%8 ) == 0 )                  /*выровн€ть полученное    */
        {
            BAS_ADDR = J;                                 /*значение на границу     */
            BAS_IND  = 0;
        }
        else                                            /*двойного слова и запомн.*/
        {
            BAS_ADDR = ( ( J >> 3 ) + 1 ) << 3;           /*его в перем.BAS_ADDR,а  */
            BAS_IND = 8 - J0;                             /*соотв.индекс масс.OBLZ-в*/
        }                          /*перем.BAS_IND           */
        
        for ( I = 0; I < IOBJC; I++ )                   /*перебира€ все считанные */
        {                                              /*карты об'ектных файлов, */
            if ( !memcmp ( &OBJCARD [I][1] , "TXT" , 3 ) )/*отобрать принадл.к типу */
            {                                            /*TXT и расчитать:        */
                memcpy ( TXT.BUF_TXT , OBJCARD [I] , 80 );  /*                        */ //80
                J = TXT.STR_TXT.ADOP [0];                   /* в переменной J начальн.*/
                J = (J << 8) + TXT.STR_TXT.ADOP [1];        /*  индекс загрузки в мас-*/
                J = (J << 8) + TXT.STR_TXT.ADOP [2];        /*  сиве OBLZ             */
                J += BAS_IND;                               /*и                       */
                /*                        */
                K = TXT.STR_TXT.DLNOP [0];                  /* в переменной K длину   */
                K = (K << 8) + TXT.STR_TXT.DLNOP [1];       /* загружаемых данных     */
                
                for ( N=0; N < K; N++ )                     /*загрузить данные с очер.*/
                    OBLZ [ (int) J++ ] = TXT.STR_TXT.OPER [N]; /*об'ектной карты         */
            }
        }
        
        
        
        
        InitCurses();
        
        res = sys();
        
        switch (res)
        {
            case 6:
            {
                endwin();
                goto ERR6;
            }
            case 7:
            {
                endwin();
                goto ERR7;
            }
            case 8:
            {
                endwin();
                goto ERR8;
            }
        }
        
        endwin();
    END:
        printf ("\n%s\n", "zavershenie obrabotki");
        
        return 0;
        //Ѕ Ћ ќ    выдачи диагностических сообщений
    ERR1:
        printf ("%s%s\n", "oshibka otkritiya faila so spiskom sobiraemix ", "modulei");
        goto END;
        
    ERR2:
        printf ("%s\n", "pystoi file so spickom sobiraemix modulei");
        goto END;
        
    ERR3:
        printf ("%s: %s\n" ,
                "oshibka otkritiya faila" , SPISOK [I] );
        goto END;
        
    ERR4:
        printf ("%s\n" ,
                "perepolnenie spiska sobiraemix modulei" );
        goto END;
        
    ERR5:
        printf ("%s\n" ,
                "perepolnenie bufera chranenia obiektnix cart");
        goto END;
        
    ERR6:
        printf ("%s\n" ,
                "nedopystimiy code comandi" );
        goto END;
        
    ERR7:
        printf("prerivanie - oshibka v adresacii \n");
        goto END;
        
    ERR8:
        goto END;
        
    ERR9:
        printf ( "%s\n", "neverniy tip faila s isxodnim textom" );
        goto END;
    }
