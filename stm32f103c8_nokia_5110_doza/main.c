

#include "main.h"
#include <stm32f10x_adc.h>
#include <stm32f10x_flash.h>
#include <stm32f10x_iwdg.h>
#include <stm32f10x_rtc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <core_cm3.h>
#include "stm32f10x_conf.h"
#include "stm32f10x_tim.h"

#include "adc.h"
#include "delay.h"
#include "ds18b20.h"
#include "flash.h"
#include "bat.h"
#include "rad_16x16.h"
#include  "celsiy_8x8.h"
#include "key.h"
#include "nokia_5110.h"
#include "prtf.h"
#include "time.h"

//void TIM3_IRQHandler ();//обработка прерываний по таймеру 3
	//void TIM2_IRQHandler ();//обработка прерываний по таймеру 2
	void RTC_IRQHandler(void);//перерывание 1с+ 1/2с таймер
	void TIM4_IRQHandler (void);
	u8  men = 0;  //номера страници
	float TCONTR;  //придел температуры
	//float w = 0;

    //calib = 0.11;
   // #define adc_calinc  0.01;
    u16 cstart = 0;

    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
    uint32_t timer = 0;//1384850400+14400;
    uint32_t settime;
    void conv_dir (u8 w,char c );
    _Bool S; //тикаем точками
    u8 S_dat;//установка даты
    u8 list=0;//системные настройки
    u16 adc_calib;


int main(void)
{

	delay_init(72);
	key_ini();// настройка портов кнопок и зв сиг
	gpio_spi_Init();
	init_lcd_5110 ();
	Init_Timer4 ();//мигаемточками
	blinc_init();
	adc_init ();//астройка ацп 1 канал  и порта ј 1
	RTC_Configuration(); //настройка часов
	Init_USART_GPIO_DS18B20();
//**********************************************************************//
	 //проверяем установку времени,если пусто подкидываем контрольное значение
	 	//timer = RTC_GetCounter();    //вачдок таймер нинт
	 	//if(timer < tm_def){/*iwdt_init (4,4000);*/RTC_SetCounter_(tm_def);}//пока вачдог не нужен
		read_seatings();
	 	if(adc_calib < 50||adc_calib >30000){adc_calib = 60; write_seatings(); }
	 	//if(!adc_calib_v){adc_calib_v = 500;}
	 	//перезаписываем по умолчанию


//*********************************************************************//


	//Get_CFG_USART_DS18B20 (0,data);

	lcd_set_strs(pos_hour,"00:00", 0);
	lcd_set_strs(pos_mday,"00/00/0000",0);
	lcd_set_strs(pos_lin,"M",0);
	lcd_set_strs(pos_lout,"У",0);
	lcd_set_strs(pos_leng,"D",0);


    while(1)
    {

    clock();//	рисуем  время и календарь
    //delay_ms(450);

if(S){lcd_set_strs(1,34,24,":", 0);}//рисуем секундные точки
		else
		{ 	lcd_set_rect(1,40,4,1);//убираем точки
		    lcd_set_rect(2,40,4,1);
		    lcd_set_rect(3,40,5,1);
		}
   //*******батарея**********//

    	d =  readADC1_W(ADC_Channel_1,adc_calib);
    	_sprtffd(2,buf,d);
    	chek_str(buf,4);          //ограничиваеи до 4 символов
    	lcd_set_strs(pos_nap,buf,0);
    	lcd_set_strs(pos_v,"V",0);
    	//bat_set(pos_bat,5, 0);
    	// bat_V_set(0,60,5, 0);

//*****************температура двиг***************************//

  if(temp_eng.code[0] == 0x28){ //если код 0 то пропускаем действае
    GET_RAM_USART_DS18B20(temp_eng.code,data);  //забираем всю память
    	    	 //  R = (crc_check(data));
    	     temp_eng.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
    	     _sprtffd(1,buf,temp_eng.d);

    	    lcd_set_strs(pos_eng,buf,0);
    	    lcd_set_sector (pos_t_eng,8,celsiy_8x8,0);
  } else {lcd_set_strs(pos_eng,"----",0);}//датчик не установлен
//******************температура в******************************//

  if(temp_in.code[0] == 0x28){ //если код 0 то пропускаем действае
    GET_RAM_USART_DS18B20(temp_in.code,data);  //забираем всю память
    	 //  R = (crc_check(data));
    	   temp_in.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
    	   _sprtffd(1,buf,temp_in.d);

    	   lcd_set_strs(pos_in,buf,0);
    	   lcd_set_sector (pos_t_in,8,celsiy_8x8,0);
  } else {lcd_set_strs(pos_in,"----",0);}//датчик не установлен
 //******************температура наруже******************************//
  if(temp_out.code[0] == 0x28){ //если код 0 то пропускаем действае
    GET_RAM_USART_DS18B20(temp_out.code,data);  //забираем всю память
    	       	 //  R = (crc_check(data));
    	   temp_out.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
    	       	   _sprtffd(1,buf,temp_out.d);
    	   lcd_set_strs(pos_out,buf,0);
    	   lcd_set_sector (pos_t_out,8,celsiy_8x8,0);
   } else {lcd_set_strs(pos_out,"----",0);}//датчик не установлен



//********************кнопочки*******************************//
/*
    	   temp_out.d = temp_out.d-240;
           oscl[i_oscl]	= oscl_tr (temp_out.d, 1);

*/
    	   key_st(10);
    	    //   if	(keys==1 & S_dat==0){ timer = RTC_GetCounter();timer+=3600; RTC_SetCounter_(timer);}

    	       if	(keys==2) { S_dat++; if(S_dat>5){ S_dat = 0;BipStop();}}//менюшка установки время календарь, однократное нажатие
    	       if	(keys==20) { list++; S_dat=0;lcd_clear();delay_ms(300); if(list>4) list = 0; }  //менюшка установки системные настройки длительное нажатие


//*******************настройка вольтметра********************************//
   while(list==1)
{
	   lcd_set_strs(0,10,8,"КАЛЛИБРОВКА",0);
	   lcd_set_strs(1,14,8,"ВОЛЬТМЕТРА",0);
	 if (keys==1){adc_calib +=(adc_calib/50);}
	 if (keys==3){adc_calib -=(adc_calib/50);}

	 d =  readADC1_W(ADC_Channel_1,adc_calib);
	     	_sprtffd(2,buf,d);
	     	chek_str(buf,4);
	     	//ограничиваеи до 4 символов
	     	lcd_set_strs(3,22,16,buf,0);
	     	lcd_set_strs(3,54,16,"V",0);

	     	delay_ms(100);
	     	key_st(10);
	     	if (meny){list++; lcd_clear();}//выходим

}
//************************поис датчика 1 **********************************************************//
   while(list==2)
   {
	  set_d_ds18b20 ("DS18   ВНЕШНИЙ");

	  	  	  	  	  if(yes)
	         					{unsigned char i;
	         				for(i=0;i<sizeof(temp_out.code);i++)
	         				{temp_out.code[i] = code[i];}
	         				save_ok();
	         				}

	         			// если нет удаляем полученное значение
	         			if(no)
	         			       {unsigned char i;
	         			       	for(i=0;i<8;i++)
	         			       	{temp_out.code[i] = 0; code[i]=0;}//обнуляем коды устройств
	         			     no_save();
	         			       	}

	         				key_st(10);
   }

//************************поис датчика 2 **********************************************************//
   while(list==3)
     {
  	  set_d_ds18b20 ("DS18  В  НУТРИ");

  	  	  	  	  	  if(yes)
  	         					{unsigned char i;
  	         				for(i=0;i<sizeof(temp_in.code);i++)
  	         				{temp_in.code[i] = code[i];}
  	         				save_ok();
  	         				}

  	         			// если нет удаляем полученное значение
  	         			if(no)
  	         			       {unsigned char i;
  	         			       	for(i=0;i<8;i++)
  	         			       	{temp_in.code[i] = 0; code[i]=0;}//обнуляем коды устройств
  	         			     no_save();
  	         			       	}

  	         				key_st(10);
     }


//*********************поисk датчика 3************************************************************************//
   while(list==4)
        {
     	  set_d_ds18b20 ("DS18 ДВИГАТЕЛЬ");

     	  	  	  	  	  if(yes)
     	         					{unsigned char i;
     	         				for(i=0;i<sizeof(temp_eng.code);i++)
     	         				{temp_eng.code[i] = code[i];}
     	         				save_ok();
     	         				}

     	         			// если нет удаляем полученное значение
     	         			if(no)
     	         			       {unsigned char i;
     	         			       	for(i=0;i<8;i++)
     	         			       	{temp_eng.code[i] = 0; code[i]=0;}//обнуляем коды устройств
     	         			     no_save();
     	         			       	}

     	         				key_st(10);
        }

   if(list==5){BipStop(); lcd_set_strs(2,16,8,"СОХРАНЯЕМ",0);
    write_seatings(); delay_ms(200); list=0;}

//**************while*********************************************************//


    }
}
//*****************************************************************************************************//

//******************************************************************************************************//
void conv_dir (u8 w, char c ){//длинна знакоместа, заполнение пустого места 0 или пробел
		   	   	   	   u8 i=0; //********длинна слова
	    	 			//u8 cl[10];//**временный массивoo
	    	 			while(buf[i]!=0)// определяем длинну строки
	    	 			{i++;}

	    	 		if(i < w){buf[w+1]=0;
	    	 			while(i>0)// конвертируем направление
	    	 			{w--;i--; buf[w]= buf[i];}
	    	 			while(w>0){w--; buf[w] = c;}
	    	 				}
	  					}
//*******************************************************************************************************//
void RTC_IRQHandler(void)//часики вывод по прерыванию 1с
 {
     if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
     {
         //* Clear the RTC Second interrupt
         RTC_ClearITPendingBit(RTC_IT_SEC);
         timer = RTC_GetCounter();           //Получаем значение счётчика
         RTC_WaitForLastTask();
     }

     IWDG_ReloadCounter();//перезапуск вачдога
 }

//*****************мигаем точками***********************************************************************************//
void TIM4_IRQHandler (void)
{     if(TIM_GetITStatus(TIM4, TIM_IT_Update)== SET)

      	{     TIM_ClearITPendingBit (TIM4, TIM_IT_Update );

      			if(S)    S=0;
      			else S=1;

      	} IWDG_ReloadCounter();//перезапуск вачдога

}



//************************выгружаем  время*******************************************************************************//
 void clock()
 {
	    	timer_to_cal (timer, &unix_time);//переобразуеи код в переменные времени часы минуты годы

			_sprtffd(0,buf,unix_time.mday);
	    	conv_dir (2,'0');
	 if(S_dat==3) {lcd_set_strs(pos_mday,buf,1);if(keys==1){timer+= 86400; RTC_SetCounter_(timer);} if(keys==3){timer-= 86400; RTC_SetCounter_(timer);} }
		else lcd_set_strs(pos_mday,buf,0);

	    	_sprtffd(0,buf,unix_time.mon);
	    	conv_dir (2,'0');
	 if(S_dat==4) {lcd_set_strs(pos_mon,buf,1); if(keys==1){unix_time.mon++; timer = cal_to_timer (&unix_time); }
	 	 	 	 	 	 	 	 	 	 	 	if(keys==3){unix_time.mon--; timer = cal_to_timer (&unix_time); }
	 	 	 	 	 	 	 	 	 	 	 	RTC_SetCounter_(timer); }
	 	 else lcd_set_strs(pos_mon,buf,0);

	 	 	 _sprtffd(0,buf,unix_time.year);
	 	    	conv_dir (2,'0');
	if(S_dat==5) {lcd_set_strs(pos_year,buf,1); if(keys==1){unix_time.year++; timer = cal_to_timer (&unix_time); }
	 	    		 	 	 	 	 	 	 	if(keys==3){unix_time.year--; timer = cal_to_timer (&unix_time); }
	 	    		 	 	 	 	 	 	 	RTC_SetCounter_(timer); }
	 	 	 else lcd_set_strs(pos_year,buf,0);


	    	 _sprtffd(0,buf,unix_time.hour);
	    	 conv_dir (2,'0');

	 if(S_dat==1) {lcd_set_strs(pos_hour,buf,1);if(keys==1) {timer+=3600; }
	  	  	  	  	  	  	  	  	  	  	  	 if(keys==3) {timer-=3600; }
	  	  	  	  	  	  	  	  	  	  	  	  	RTC_SetCounter_(timer); }
	 	 	 else  lcd_set_strs(pos_hour,buf,0);

	    	 _sprtffd(0,buf,unix_time.min);
	    	 conv_dir (2,'0');

	  if(S_dat==2) {lcd_set_strs(pos_min,buf,1);if(keys==1) {timer+=60; }
	    	 	  	  	  	  	  	  	  	  	 if(keys==3) {timer-=60; }
	    	 	  	  	  	  	  	  	  	  	  	 RTC_SetCounter_(timer); }
	  	  	  else  lcd_set_strs(pos_min,buf,0);


	    	   switch(unix_time.wday)
	    	   					{
	    	   				case 0 : lcd_set_strs(pos_dn,"ПОН",0);break;
	    	   				case 1 : lcd_set_strs(pos_dn,"ВТР",0);break;
	    	   				case 2 : lcd_set_strs(pos_dn,"СРД",0);break;
	    	   				case 3 : lcd_set_strs(pos_dn,"ЧТВ",0);break;
	    	   				case 4 : lcd_set_strs(pos_dn,"ПТН",0);break;
	    	   				case 5 : lcd_set_strs(pos_dn,"СБТ",0);break;
	    	   				case 6 : lcd_set_strs(pos_dn,"ВСК",0);break;
	    	   				default : return;
	    	   					}

 }


//**********************подключение датчиков**************************************************************************//

 void set_d_ds18b20 (unsigned char set_d[16])
  {int i;


     		if(Reset_USART_DS18B20())
     		{//забираем память
     		Get_Rom_USART_DS18B20(code);

     		for(i=0;i<8;i++){_sprtf16(data,code[i]); buf[i*2]= data[0];
     						buf[i*2+1]= data[1];
     						buf[i*2+2]= 0;}


     		lcd_set_strs(0,0,8,set_d,0);
     		lcd_set_strs(2,36,8,"ID",0);
     		lcd_set_strs(3,0,8,buf,0);
     		lcd_set_strs(5,0,8,"сохр",0);
     		//lcd_set_strs(5,36,8,"следу",0);
     		lcd_set_strs(5,64,8,"нет",0);

     		delay_ms(100);

     		}
     		else {lcd_clear(); lcd_set_strs(4,4,16,"ERROR SERCH",1);
     		delay_ms(500); }

  }


//************************сохранил*************************************************************************//


 void save_ok ()  	  	{
	 lcd_clear();
   	lcd_set_strs(2,28,8,"ГОТОВ!",0);
      delay_ms(1000); list++;lcd_clear();

  }

 //********************************************************************************************************//
 void no_save ()
 {	lcd_clear();
	 lcd_set_strs(2,2,8,"НЕ УСТАНОВЛЕН!",0);
       delay_ms(1000);list++;lcd_clear();
 }
 //**********************************************************************************************************//
 //*****************************************************************************************************//
/* void Write_settings_to_flash ()
  {	u16 F[3];
  	F[0]= adc_calib;
  	//F[1]= adc_calib_v;
  	//F[2]= 0;
  	//F[3]= cstart;
  	FLASH_WriteHalfDate(0x0800fC00, F,3);//запись во флэш
  }
  void Read_settings_to_flash ()
  {
  	u16 F[3];
  	FLASH_ReadHalfData(0x0800fC00 , F,3);//чтение  флэш
  	adc_calib =      F[0];
  	//adc_calib =      F[1];
  }
 */
 /****************************************************************************************/
 	void write_seatings()
 	{
 		    	FLASH_Unlock();
 		         FLASH_ClearFlag (FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//С‡РёСЃС‚РёРј С„Р»РіРё
 		           FLASHStatus = FLASH_ErasePage (adr_start);
 		           adr = adr_start;//назначаем адресс начала данных
 		           FLASH_Write (temp_in.code,sizeof(temp_in.code));
 		           FLASH_Write (temp_out.code,sizeof(temp_out.code));
 		           FLASH_Write(temp_eng.code,sizeof(temp_eng.code));
 		           FLASHStatus = FLASH_ProgramHalfWord(adr, adc_calib);
 		        FLASH_Lock();
 		    }

 //*****************************************************************************************//
 	void FLASH_Write (uint16_t *p, u8 R)
 	{
 		while  (R--)
 				        { IWDG_ReloadCounter();
 				        FLASHStatus = FLASH_ProgramHalfWord(adr, *p++); adr +=2;}
 	}

 /*****************************************************************************************/
 	void read_seatings()
 	{	adr = adr_start;//назначаем адресс начала данных

 		read_Write(temp_in.code, sizeof(temp_in.code));

 		read_Write(temp_out.code, sizeof(temp_out.code));

 		read_Write(temp_eng.code, sizeof(temp_eng.code));

 		adc_calib=*((uint16_t*)adr);

 	if(temp_eng.code[0] == 0x28){
 		//Reset_USART_DS18B20();//контроль темпер воды
 		Get_CFG_USART_DS18B20(temp_eng.code,data);
 		convert_atl_ath ();
 	    temp_eng.TCONTR = CONV_TEMP_DS18B20 (0.625,TH_,TL_);	}// контроль тепмпер пересчитавеем по приращению

 	if(temp_in.code[0] == 0x28){ //если код 0 то пропускаем действае
 		//Reset_USART_DS18B20();//контроль темпер воды
 		Get_CFG_USART_DS18B20(temp_in.code,data);
 		convert_atl_ath ();
 	    temp_in.TCONTR = CONV_TEMP_DS18B20 (0.625,TH_,TL_);}	// контроль тепмпер пересчитавеем по приращению
 	}
 //*****************************************************************************************//
 	void read_Write(uint16_t *p, u8 R)
 	{
 		 while(R--)
 			    {IWDG_ReloadCounter();//перезапуск вачдога
 			     *(p++)=*((uint16_t*)adr); adr+=2;
 			   }
 	}

 	//*************************************************************************************************//
 	 void convert_atl_ath ()
 	 {
 		 	 	 TL_ = ATH_;
 		      	  TL_ <<= 4;
 		 		  TH_= ATH_;
 		 		  TH_ >>= 4 ;
 		 	 if(ATH_ > 0x7D){TH_ |= 0xF8;}
 	 }

 	//************************************************************************************************//
