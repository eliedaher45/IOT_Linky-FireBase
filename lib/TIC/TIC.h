#pragma once
#include <Arduino.h>

enum MODE{
  Historique,
  Standard
};

enum MODEPHASE{
  Mono,
  Triphase
};

const int BD[] = {1200,9600};
const int WAIT_BUFF[] = {3000,600};

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

void getInfo_mono(char[]);
void getInfo_tri(char[]);
void setDone(bool);

bool is_Done();
bool getParity(unsigned int n);
bool checksum(String data,char chk);

String getJson();