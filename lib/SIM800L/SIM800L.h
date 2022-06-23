#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>

#define USE_SSL true
#define DELAY_MS 500

extern SoftwareSerial SIM800;

const String APN  = "TM";   
const String USER = "";
const String PASS = "";

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

// ELIE test DB
// const String FIREBASE_HOST  = "https://berangertest-default-rtdb.europe-west1.firebasedatabase.app/";
// const String FIREBASE_SECRET  = "U4M9HM3b4oKS4i88HsXVGvYosC3W4DcL7rVmDJOd";
// const String FIREBASE_DIRECTORY = "IoT";

//BERANGER DB
const String FIREBASE_HOST  = "https://maintenanceberanger-default-rtdb.firebaseio.com/";
const String FIREBASE_SECRET  = "c0bjHT3VPop0Pf06JawoHQSY3LM9nR6UHXylO8uB";
const String FIREBASE_DIRECTORY = "Results";

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

void init_gsm();
void gprs_connect();
void post_to_firebase(String data);
bool SIM_Ready();
bool gprs_disconnect();
bool is_gprs_connected();
bool waitResponse(String expected_answer="OK", unsigned int timeout=2000);

String getTime(String &data);
String getResponse();