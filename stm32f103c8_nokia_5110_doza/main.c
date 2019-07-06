

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
#include "stm32f10x_exti.h"

#include  "chip_clock.h"
#include "adc.h"
#include "delay.h"
#include "ds18b20.h"
#include "flash.h"
#include "bat.h"

#include "celsiy_8x8.h"
#include "key.h"
#include "nokia_5110.h"
#include "prtf.h"
#include "time.h"


	void RTC_IRQHandler(void);//перерывание 1с+ 1/2с таймер
	void TIM4_IRQHandler (void);//прерывание для секундных точек
	void  EXTI15_10_IRQHandler(void);//прерывания для кнопок
//	void  EXTI9_5_IRQHandler(void);//прерывания для кнопок
	u8  men = 0;  //номера страници
	float TCONTR;  //придел температуры
	//float w = 0;
	u8 L;//длинна переменной
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
    u8 list_c=0;//вид основного окна
    u16 adc_calib;


int main(void)
{

	sysclock = 8;//значение используется в настройке таймеров и задержек
	RCC_SYSCLKConfig( RCC_SYSCLKSource_HSE);
	RCC_HCLKConfig( RCC_SYSCLK_Div2);
	RCC_PLLCmd(DISABLE);
	delay_init(sysclock);//уменьшаем скорость для экономии энерги 8м

	//delay_init(72);
	key_ini();// настройка портов кнопок и зв сиг
	gpio_spi_Init();
	init_lcd_5110 ();
	Init_Timer4 ();//мигаемточками
	blinc_init();
	adc_init ();//астройка ацп 1 канал  и порта ј 1
	RTC_Configuration(); //настройка часов
	Init_USART_GPIO_DS18B20();

		ILL_init();
		//ILL_ON();
		read_seatings(); //читаем настройки
		if(level_ill>20){level_ill=5;}
		level (level_ill);// устанавливаем яркость

//**********************************************************************//
	 //проверяем установку времени,если пусто подкидываем контрольное значение
	 	timer = RTC_GetCounter();
	 	if(timer < tm_def ){RTC_SetCounter_(tm_def);}//пока вачдог не нужен
	 	if(adc_calib < 2||adc_calib >60000){adc_calib = 60; write_seatings(); }
	 	//if(!adc_calib_v){adc_calib_v = 500;}
	 	//перезаписываем по умолчанию

	 //	iwdt_init (256,60000);//ВАЧДОГ
//*********************************************************************//

	 	set_def_list();

    while(1)
    {  IWDG_ReloadCounter();//перезапуск вачдога

    clock();//	рисуем  время и календарь
    //delay_ms(450);
   //*********ILL**************//

    d =  readADC1_W(ADC_Channel_1,adc_calib);
    if(d>600){ILL_ON();}//примерно 9V отрабатываетё
    else {ILL_OFF();} //включать по необходимости!!!


   //*******батарея**********//

    	d =  readADC1_W(ADC_Channel_2,adc_calib);
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

//**************************СОН*********************************************************************//
    	   if(!S_dat){__WFI();}//если не в меню то спать

    	 //**спаль до прерывания по таймеру сек или  индикации
    	 if(S){lcd_set_strs(1,34,24,":", 0);}//рисуем точки и спать
    	 	 else
    	       	   {
    	 		 	 lcd_set_rect(1,40,4,1);//убираем точки и спать
				    lcd_set_rect(2,40,4,1);
				    lcd_set_rect(3,40,5,1);
				    if(!S_dat){__WFI();}//если не в меню то спать
				    }
//********************кнопочки*******************************//

    	   key_st(10);

    	       if	(keys==2) {IWDG_ReloadCounter(); S_dat++; if(S_dat>5){ S_dat = 0;BipStop();}}//менюшка установки время календарь, однократное нажатие
    	       if	(keys==20) { IWDG_ReloadCounter(); list++; S_dat=0; lcd_clear(); delay_ms(300);}  //менюшка установки системные настройки длительное нажатие


//*******************************************************************************************************************************************//
//*************************************************************************************************//

//*******************настройка яркости********************************//
    if(list==1)
    {
    	ILL_ON(); //включаем свет

	  while (list==1)
	  {
  	  	  	  //L=(84-7*6)/2;
    	       lcd_set_strs(0,21,8,"ЯРКОСТЬ",0);
   	       	   	  // L=(84-9*6)/2;
    	       lcd_set_strs(1,15,8,"ПОДСВЕТКИ",0);

    	       	if	(keys==1){level_ill++;if(level_ill>20)level_ill=0;    level ( level_ill ); IWDG_ReloadCounter(); }
    	       	if	(keys==3){level_ill--;if(level_ill>20)level_ill=20;   level ( level_ill ); IWDG_ReloadCounter(); }

    	       	lcd_set_strs(5,0,8,"+",0);
    	       	lcd_set_strs(5,30,8,"меню",0);
    	       	lcd_set_strs(5,78,8,"-",0);

    	       	     	_sprtffd(0,buf,level_ill);
    	       	         conv_dir (2,'0');
    	       	     	lcd_set_strs(2,26,24,buf,0);

    	       	     	delay_ms(50);
    	       	     	key_st(10);
    	       	     	meny_driv ();//двигаем менюшку или выходим
	  }
    }
//*******************настройка вольтметра********************************//
   while(list==2)
{
	   	   	   	   	  lcd_set_strs(5,0,8,"+",0);
	       	       	lcd_set_strs(5,30,8,"меню",0);
	       	       	lcd_set_strs(5,78,8,"-",0);
	   //L=(84-11*6)/2;
	   lcd_set_strs(0,9,8,"КАЛЛИБРОВКА",0);
	   	   	//   L=(84-10*6)/2;
	   lcd_set_strs(1,12,8,"ВОЛЬТМЕТРА",0);
	 if (keys==1){adc_calib +=(adc_calib/50); IWDG_ReloadCounter();}
	 if (keys==3){adc_calib -=(adc_calib/50); IWDG_ReloadCounter();}

	 d =  readADC1_W(ADC_Channel_2,adc_calib);
	     	_sprtffd(2,buf,d);
	     	chek_str(buf,4);
	     	//ограничиваеи до 4 символов
	     	lcd_set_strs(2,10,24,buf,0);
	     	lcd_set_strs(3,74,16,"V",0);

	     	delay_ms(100);
	     	key_st(10);
	     	meny_driv ();//двигаем менюшку или выходим

}
//************************поис датчика 1 **********************************************************//
   while(list==3)
   {
	 // если есть данные выводим

	   key_st(10);



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
	         			if(meny){messag("СЛЕДУЮЩИЙ");}


   }  IWDG_ReloadCounter();

//************************поис датчика 2 **********************************************************//
   while(list==4)
     {

	   key_st(10);
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
  	         			if(meny){messag("СЛЕДУЮЩИЙ");}

     } IWDG_ReloadCounter();


//*********************поисk датчика 3************************************************************************//
   while(list==5)
        {
	   key_st(10);

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

     	         			if(meny){messag("СЛЕДУЮЩИЙ");}

        } IWDG_ReloadCounter();

   if(list==6){BipStop(); lcd_set_strs(2,16,8,"СОХРАНЯЕМ",0);
    write_seatings(); delay_ms(200); list=0;set_def_list();
    		ILL_OFF;}

//**************while*********************************************************//


    }
}
//*****************************************************************************************************//
//*******************************************************************************************************//
void meny_driv() {
	if (meny){list++; lcd_clear();}//следующее меню//   L=(84-9*6)/2;
   	       	     if(meny_l){IWDG_ReloadCounter(); BipStop(); lcd_clear();lcd_set_strs(2,15,8,"СОХРАНЯЕМ",0);
   	      	        write_seatings(); delay_ms(200); list=0;lcd_clear(); set_def_list();}//сохраняем и выходим
   	       	 }


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

         TIM_Cmd (TIM4, ENABLE);
     }

     IWDG_ReloadCounter();//перезапуск вачдога
 }

//*****************мигаем точками***********************************************************************************//
void TIM4_IRQHandler (void)
{     if(TIM_GetITStatus(TIM4, TIM_IT_Update)== SET)

      	{
      			if(S)    S=0;
      			else S=1;
      			TIM_Cmd (TIM4, DISABLE);
      			TIM_ClearITPendingBit (TIM4, TIM_IT_Update );

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

	 if(S_dat==2) {lcd_set_strs(pos_hour,buf,1);if(keys==1) {timer+=3600; }
	  	  	  	  	  	  	  	  	  	  	  	 if(keys==3) {timer-=3600; }
	  	  	  	  	  	  	  	  	  	  	  	  	RTC_SetCounter_(timer); }
	 	 	 else  lcd_set_strs(pos_hour,buf,0);

	    	 _sprtffd(0,buf,unix_time.min);
	    	 conv_dir (2,'0');

	  if(S_dat==1) {lcd_set_strs(pos_min,buf,1);if(keys==1) {timer+=60; }
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
//********************************************************************************************************************//
 //************************выгружаем  время*******************************************************************************//
  void clock_2()
  {
 	    	timer_to_cal (timer, &unix_time);//переобразуеи код в переменные времени часы минуты годы

 	    	 _sprtffd(0,buf,unix_time.hour);
 	    	 conv_dir (2,'0');

 	 if(S_dat==2) {lcd_set_strs(0,12,16,buf,1);if(keys==1) {timer+=3600; }
 	  	  	  	  	  	  	  	  	  	  	  	 if(keys==3) {timer-=3600; }
 	  	  	  	  	  	  	  	  	  	  	  	  	RTC_SetCounter_(timer); }
 	 	 	 else  lcd_set_strs(0,12,16,buf,0);

 	    	 _sprtffd(0,buf,unix_time.min);
 	    	 conv_dir (2,'0');

 	  if(S_dat==1) {lcd_set_strs(0,40,16,buf,1);if(keys==1) {timer+=60; }
 	    	 	  	  	  	  	  	  	  	  	 if(keys==3) {timer-=60; }
 	    	 	  	  	  	  	  	  	  	  	  	 RTC_SetCounter_(timer); }
 	  	  	  else  lcd_set_strs(0,40,16,buf,0);


  }


//**********************подключение датчиков**************************************************************************//

 void set_d_ds18b20 (unsigned char set_d[16])//set_d ПИШЕМ ЗАГАЛОВОК
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
     		lcd_set_strs(5,30,8,"меню",0);
     		lcd_set_strs(5,66,8,"удл",0);

     		delay_ms(100);

     		}
     		else {lcd_clear(); lcd_set_strs(2,4,16,"ERROR SERCH",1);
     		delay_ms(400); }

  }




//************************сохранил*************************************************************************//


 void save_ok ()  {  	 IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-6*6)/2;
   	lcd_set_strs(2,24,8,"ГОТОВ!",0);
      delay_ms(1000); list++;lcd_clear();

  }

 //********************************************************************************************************//
 void no_save ()
 {	IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-14*6)/2;
	 lcd_set_strs(2,2,8,"НЕ УСТАНОВЛЕН!",0);
       delay_ms(1000);list++;lcd_clear();
 }
 //**********************************************************************************************************//

 void messag (unsigned char mess[16])
  {	IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-9*6)/2;
 	 lcd_set_strs(2,15,8,mess,0);
        delay_ms(1000);list++;lcd_clear();
  }
 //***********************************************************************************************************//
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
 		IWDG_ReloadCounter();
 		FLASH_Unlock();
 		         FLASH_ClearFlag (FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//С‡РёСЃС‚РёРј С„Р»РіРё
 		           FLASHStatus = FLASH_ErasePage (adr_start);
 		           adr = adr_start;//назначаем адресс начала данных


 		           FLASH_Write (temp_in.code,sizeof(temp_in.code));
 		           FLASH_Write (temp_out.code,sizeof(temp_out.code));
 		           FLASH_Write(temp_eng.code,sizeof(temp_eng.code));
 		          FLASHStatus = FLASH_ProgramHalfWord(adr, level_ill);adr+=2;
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
 	{
 		IWDG_ReloadCounter();
 		adr = adr_start;//назначаем адресс начала данных


 	FLASH_read(temp_in.code, sizeof(temp_in.code));
 	FLASH_read(temp_out.code, sizeof(temp_out.code));
 	FLASH_read(temp_eng.code, sizeof(temp_eng.code));
  	level_ill=*((uint16_t*)adr);adr+=2;
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
 	void FLASH_read(uint16_t *p, u8 R)
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
 	void set_def_list()
 	{
 		IWDG_ReloadCounter();
 		lcd_set_strs(pos_hour,"00:00", 0);
 			lcd_set_strs(pos_mday,"00/00/0000",0);
 			lcd_set_strs(pos_lin,"M",0);
 			lcd_set_strs(pos_lout,"У",0);
 			lcd_set_strs(pos_leng,"D",0);

 	}
 	 //***********************************************************************************************//
 	void set_2t()					//рисуем секундные точки
 	{
 		if(S){lcd_set_strs(1,34,24,":", 0);}
 				else
 				{ 	lcd_set_rect(1,40,4,1);//убираем точки
 				    lcd_set_rect(2,40,4,1);
 				    lcd_set_rect(3,40,5,1);
 				}
 	}
//***********************************************************************************************************//
 	void  EXTI15_10_IRQHandler(void)
 	 {
 	 	 	 EXTI_ClearITPendingBit(EXTI_Line10|EXTI_Line11|EXTI_Line12);

 	 }

 /*	void  EXTI9_5_IRQHandler(void)
 	 	 {
 	 	 	 	 EXTI_ClearITPendingBit(EXTI_Line9);

 	 	 }*/
//***********************************************************************************************************//



//************************************************************************************************************//
