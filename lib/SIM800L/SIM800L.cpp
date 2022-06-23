#include "SIM800L.h"
#include "config.h"

SoftwareSerial SIM800(rxPin,txPin);

bool Ready = false;

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Initialize GSM Module //
void init_gsm()
{
  // Testing AT Command //
  SIM800.println("AT");
  waitResponse();
  HAL_Delay(DELAY_MS);

  // Checks if the SIM is ready //
  SIM800.println("AT+CPIN?");
  waitResponse("+CPIN: READY");
  HAL_Delay(DELAY_MS);
  
  // Turning ON full functionality //
  SIM800.println("AT+CFUN=1");
  waitResponse();
  HAL_Delay(DELAY_MS);

  // Turn ON verbose error codes //
  SIM800.println("AT+CMEE=2");
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Enable battery checks //
  SIM800.println("AT+CBATCHK=1");
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Register Network (+CREG: 0,1 or +CREG: 0,5 for valid network) //
  // +CREG: 0,1 or +CREG: 0,5 for valid network connection //
  SIM800.println("AT+CREG?");
  waitResponse("+CREG: 0,");
  HAL_Delay(DELAY_MS);
  
  // Setting SMS text mode //
  SIM800.print("AT+CMGF=1\r");
  if(waitResponse("OK"))
    Ready = true;
  HAL_Delay(DELAY_MS);

  // Syncing GSM Time //
  SIM800.print("AT+CLTS=1\n");
  waitResponse("OK");
  SIM800.print("AT&W");
  waitResponse("OK");
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Return if Init Sim commands done and SIM ready //
bool SIM_Ready()
{
  return Ready;
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Get Time from GSM //
String getTime(String &data)
{
  SIM800.println("AT+CCLK?");
  String val_Time = getResponse();
  data += val_Time + "\"}";
  return val_Time;
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Post data to FireBase //
void post_to_firebase(String data)
{
  // Start HTTP connection //
  SIM800.println("AT+HTTPINIT");
  waitResponse();
  HAL_Delay(DELAY_MS);

  // Enabling SSL 1.0 //
  if(USE_SSL == true)
  {
    SIM800.println("AT+HTTPSSL=1");
    waitResponse();
    HAL_Delay(DELAY_MS);
  }
  
  // Setting up parameters for HTTP session //
  SIM800.println("AT+HTTPPARA=\"CID\",1");
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Set the HTTP URL - Firebase URL, FireBase Secret key and FireBase Directory //
  SIM800.println("AT+HTTPPARA=\"URL\","+FIREBASE_HOST+FIREBASE_DIRECTORY+".json?auth="+FIREBASE_SECRET);
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Setting up re direct //
  SIM800.println("AT+HTTPPARA=\"REDIR\",1");
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Setting up content type //
  SIM800.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Setting up Data Size //
  // +HTTPACTION: 1,601,0 - error occurs if data length is not correct
  SIM800.println("AT+HTTPDATA=" + String(data.length()) + ",10000");
  waitResponse("DOWNLOAD");
  
  // Sending Data //
  SIM800.println(data);
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Sending HTTP POST request //
  SIM800.println("AT+HTTPACTION=1");

  for (uint32_t start = millis(); millis() - start < 20000;)
  {
    while(!SIM800.available() && millis()-start <5000); //added condition to exit
    String response = SIM800.readString();
    if(response.indexOf("+HTTPACTION:") > 0)
    {
      Serial.println(response);
      String code = response.substring(response.indexOf("+HTTPACTION:")+15,response.indexOf("+HTTPACTION:")+18);
      Serial.println("action code: " + code);
      // Checking for error in HTTP Post //
      // +HTTPACTION: 1,603,0 (POST to Firebase failed)
      // +HTTPACTION: 0,200,0 (POST to Firebase successfull)
      if ( code != "200")
        gprs_disconnect();
      break;
    }
  }
  HAL_Delay(DELAY_MS);
  
  // Read the response //
  SIM800.println("AT+HTTPREAD");
  waitResponse("OK");
  HAL_Delay(DELAY_MS);

  // Stop HTTP connection //
  SIM800.println("AT+HTTPTERM");
  waitResponse("OK",1000);
  HAL_Delay(DELAY_MS);
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Connect to the Internet //
void gprs_connect()
{
  Serial.println("gprs_connect()");
  // DISABLE Connection // 
  SIM800.println("AT+SAPBR=0,1");
  waitResponse("OK",3000); //60000);
  HAL_Delay(DELAY_MS);

  // Connecting to GPRS: GPRS - bearer profile 1 //
  SIM800.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  waitResponse();
  HAL_Delay(DELAY_MS);
  
  // Sets the APN settings for the Sim card network provider //
  SIM800.println("AT+SAPBR=3,1,\"APN\","+APN);
  waitResponse();
  HAL_Delay(DELAY_MS);
  //Sets the user name settings for the Sim card network provider.
  if(USER != ""){
    SIM800.println("AT+SAPBR=3,1,\"USER\","+USER);
    waitResponse();
    HAL_Delay(DELAY_MS);
  }
  //sets the password settings for the Sim card network provider.
  if(PASS != ""){
    SIM800.println("AT+SAPBR=3,1,\"PASS\","+PASS);
    waitResponse();
    HAL_Delay(DELAY_MS);
  }
  
  // Enable the GPRS: enable bearer 1 //
  SIM800.println("AT+SAPBR=1,1");
  waitResponse("OK", 30000);
  HAL_Delay(DELAY_MS);
  
  // Get IP Address - Query the GPRS bearer context status //
  SIM800.println("AT+SAPBR=2,1");
  waitResponse("OK");
  HAL_Delay(DELAY_MS);
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Disconnect from the Internet //
// AT+CGATT = 1 modem is attached to GPRS to a network. 
// AT+CGATT = 0 modem is not attached to GPRS to a network
bool gprs_disconnect()
{
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Disconnect GPRS
  SIM800.println("AT+CGATT=0");
  waitResponse("OK",60000);
  //HAL_Delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //DISABLE GPRS
  //SIM800.println("AT+SAPBR=0,1");
  //waitResponse("OK",60000);
  //HAL_Delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

  return true;
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Check if the GPRS is connected.
// AT+CGATT = 1 modem is attached to GPRS to a network. 
// AT+CGATT = 0 modem is not attached to GPRS to a network.
bool is_gprs_connected()
{
  SIM800.println("AT+CGATT?");
  if(waitResponse("+CGATT: 1",6000) == 0) 
    return false; 
  
  return true;
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
// Handling AT COMMANDS //
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Wait for Specific Response //
bool waitResponse(String expected_answer, unsigned int timeout) //uncomment if syntax error (esp8266)
{
  uint8_t x=0, answer = 0;
  String response;
  unsigned long previous;
    
  // Clean the input buffer //
  while( SIM800.available() > 0) SIM800.read();
  
  previous = millis();
  do{
    // if data in UART INPUT BUFFER, reads it //
    if(SIM800.available() != 0){
        char c = SIM800.read();
        response.concat(c);
        x++;
        // checks if the (response == expected_answer) //
        if(response.indexOf(expected_answer) > 0){
            answer = 1;
        }
    }
  }while((answer == 0) && ((millis() - previous) < timeout));
  
  if (expected_answer != "+CGATT: 1")
    Serial.println(response);
  return answer;
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// Get a response from the Module //
String getResponse()
{
  unsigned long previous;
  String response ="";

  previous = millis();
  
  // Clean the input buffer //
  while( SIM800.available() > 0) SIM800.read();

  do{
    if (SIM800.available())
    {
      response = SIM800.readStringUntil('"');
      response = SIM800.readStringUntil('"');
      return response;
    }
  } while (response=="" && ((millis() - previous) < 3000));
  response = "NAN";
  return response;
}