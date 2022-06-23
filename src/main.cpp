#include <Arduino.h>
#include "config.h"
#include "SIM800L.h"
#include "TIC.h"

// MAPPING UART1 PORTS for Linky
HardwareSerial Serial1(ticRx, ticTx); // RX/TX (PA10/PA9)

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

unsigned long prev;
char buff[BUFF_SIZE];

int nb;

bool Sleep = false;
bool post = false;
bool debugPost = POST;
bool mono_tri = false;

String data;
String frame;
String time;

MODE Mode;
String Modes[] = {"Historique","Standard"};
MODEPHASE ModePhase;
String MP[] = {"Monophase","Triphase"};

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

void SystemClock_Config(void);
void send();

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

void setup()
{
  pinMode(B1,INPUT);
  pinMode(BTN_MODE,INPUT_PULLUP);
  
  pinMode(LD2,OUTPUT);
  pinMode(LD_H,OUTPUT);
  pinMode(LD_S,OUTPUT);

  digitalWrite(LD2,LOW);
  
  // Loading LED.. //
  digitalWrite(LD2,HIGH);
  HAL_Delay(BOOT_WAIT);
  digitalWrite(LD2,LOW);
  
  // Get Mode //
  if (digitalRead(BTN_MODE) == LOW)
  {
    Mode = Historique;
    digitalWrite(LD_H,LOW);
    digitalWrite(LD_S,HIGH);
    // SYSCLCK = 42 MHz for Mode Historique //
    SystemClock_Config();
  }
  else
  {
    Mode = Standard;
    digitalWrite(LD_S,LOW);
    digitalWrite(LD_H,HIGH);
  }
  
  Serial.begin(57600);
  Serial.println("\r\n==========================");
  Serial.println("Mode : " + Modes[Mode]);
  
  // TIC Serial Initialisation //
  Serial1.begin(BD[Mode], SERIAL_7E1);

  // Sending Button for Debug // 
  attachInterrupt(digitalPinToInterrupt(B1),send,LOW);

  // SIM800 SERIAL Initialisation //
  SIM800.begin(9600);
  do
  {
    Serial.println("Initializing SIM800...");
    init_gsm();
  }while(!SIM_Ready());
  Serial.println("Setup Complete!");
}

void loop()
{
  // Connect to GPRS //
  while(!is_gprs_connected())
  {
    Serial.println("Trying to Connect...");
    gprs_connect();
    if(is_gprs_connected())
      Serial.println("Connected Successfuly!");
    else
      Serial.println("Failed to Connect!");
  }

  // Wait between each frame //
  if(Sleep)
  {
    Serial.println("Sleep..");
    digitalWrite(LD2, HIGH);
    HAL_Delay(WAIT_S*1000);
    digitalWrite(LD2, LOW);
    Sleep = false;
  }

  // Reading Serial Data... //
  else if (Serial1.available())
  {
    // Detecting Phase Mode: Monophase ou Triphase //
    if(!mono_tri)
    {
      static bool first = true;
      if(first)
      {
        Serial.print("Getting Phase Mode");
        first = false;
      }

      data = Serial1.readStringUntil('\n');
      Serial.print(".");

      if(data.indexOf("ADCO")>=0)
      {
        ModePhase= Mono;
        mono_tri = true;
      }
      else if ((data.indexOf("ADSC")>=0))
      {
        ModePhase = Triphase;
        mono_tri = true;
      }
      
      if(mono_tri)
        Serial.println("\r\nPhase Mode : " + MP[ModePhase]);
    }

    // Mode Known!.. Reading Data... //
    else
    {
      // Empty buffer for fresh reading //
      while(Serial1.read() >= 0)
        Serial1.flush(); 
      
      // Wait for new data to fill the buffer //
      HAL_Delay(WAIT_BUFF[Mode]);

      // read data from buffer in "buff" //
      nb = Serial1.readBytesUntil(0,buff,BUFF_SIZE);
      
      switch(ModePhase)
      {
        case Mono:
          getInfo_mono(buff);
          break;
        case Triphase:
          getInfo_tri(buff);
          break;
      }

      // If json frame complete //
      if (is_Done())
      {
        setDone(false);
        frame = getJson();
        // Concatinate Date&Time to the frame //
        time = getTime(frame).substring(9,11);
        Serial.println("\r\nJSON :\r\n"+frame);
        debugln("time(hh): "+ time);

        // Test if IDLE Time de send the frame //
        if (!(time.toInt()<=START_HOUR && time.toInt()>=END_HOUR))
        {
          post = true;
          Serial.println("Sending data to D.B...");
        }
        else
        {
          post = false;
          Serial.println("Offline time..Data will not be send!");
        }
        if (post || debugPost)
        {
          post_to_firebase(frame);
          HAL_Delay(2000);
        }
        Sleep = true;
      }
    }
  }
}

// Force-Send function for Debugging //
void send(){
  debugPost = !debugPost;
  HAL_Delay(500);
  if(debugPost)
    Serial.println("DATA will be Sent...");
  else
    Serial.println("DATA will NOT be Sent...");
}

// Config Clck to 42 MHz for Historique Mode //
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  //** Configure the main internal regulator output voltage
   
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  //** Initializes the RCC Oscillators according to the specified parameters
  //* in the RCC_OscInitTypeDef structure.
  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 21;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  //* Initializes the CPU, AHB and APB buses clocks
   
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}
