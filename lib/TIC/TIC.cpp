#include "TIC.h"
#include "config.h"

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

bool flag_P[4];
bool done = false;
bool isADCO = false;
int ind_ADCO, ind_P, ind_ADSC, ind_S;
String val_ADSC;
String val_ADCO, val_P[4];
String json_frame;
String info;

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

void getInfo_mono(char buff[])
{
  String buff_s = buff;
  if (!isADCO)
  {
    ind_ADCO = buff_s.indexOf("ADCO");
    if (ind_ADCO >=0 )
    {    
        String info = buff_s.substring(ind_ADCO,ind_ADCO+19);
        char ch = info.charAt(18);
        debugln("\r\ninfo: "+info);
        debug("checksum: "); //debugln(ch);

        if (checksum(info.substring(0,17),ch))
        {
            val_ADCO = info.substring(5,17);
            isADCO = true;
            debugln("----------------------\r\n| ADCO: " + val_ADCO+" |\r\n----------------------");
        }
        else
        debugln("Err");
    }
    else 
      debug(".");
  }
  else
  {
    ind_P = buff_s.indexOf("PAPP");
    if (ind_P >=0)
    {
        String info = buff_s.substring(ind_P,ind_P+12);
        char ch = info.charAt(11);
        debugln("\r\ninfo: "+info);
        debug("checksum: "); //debugln(ch);
        if (checksum(info.substring(0,10),ch))
        {
            val_P[0] = info.substring(5, 10);
            flag_P[0] = true;
            debugln("----------------------\r\n|   P: "+ val_P[0]+"         |\r\n----------------------");
        }
        else
            debugln("Err");
    }
    else 
        debug(".");
    
    if (flag_P[0])
    {
        json_frame = "{\"ADCO\":\"" + String(val_ADCO) + "\",\"PAPP\":\"" + String(val_P[0]) + "\",\"Time\":\"";//\"IINST\":\"" + String(val_I[0]) + "\"}";
        done = true;
        isADCO = false;
        flag_P[0] = false;
    }
  }
}

void getInfo_tri(char buff[])
{
    String buff_s = buff;
    if (!isADCO)
    {
        ind_ADSC = buff_s.indexOf("ADSC");
        if (ind_ADSC >= 0 )
        {
            flag_P[1] = flag_P[2] = flag_P[3] = false;
            String info = buff_s.substring(ind_ADSC,ind_ADSC+19);
            char ch = info.charAt(18);
            debugln("\r\ninfo: "+info);
            debug("checksum: ");
            //debugln(ch);
            if (checksum(info.substring(0,18),ch))
            {
                val_ADSC = info.substring(5,17);
                isADCO = true;
                debugln("----------------------\r\n| ADSC: "+ val_ADSC + " |\r\n----------------------");
            }
            else
                debugln("Err");
        }
        else 
            debug(".");
    }
    else
    { 
        ind_S = 0;
        for(int phase=0; phase<4; phase++)
        {
            ind_S = buff_s.indexOf("SINSTS",ind_S+1);
            if ( ind_S >= 0 )
            {
                String info = buff_s.substring(ind_S,ind_S+15);
                char ch = info.charAt(14);
                debugln("\r\ninfo: " + info);
                debugln("phase= "+String(phase));
                debug("checksum: "); //debugln(ch);

                if (checksum(info.substring(0,14),ch))
                {
                    if(info.substring(6,7)==String(phase))
                    {
                        val_P[phase] = info.substring(8,13);
                        flag_P[phase] = true;
                        debugln("----------------------\r\n| S["+String(phase)+ "] : " + val_P[phase]+"       |\r\n----------------------");
                    }
                }
                else
                    debugln("Err");
            }
            else
                debug(".");
        }

        if (flag_P[1] == true && flag_P[2] == true && flag_P[3] == true)
        {
            json_frame = "{\"ADCO\":\"" + String(val_ADSC) + "\",\"PAPP1\":\"" + String(val_P[1]) +"\",\"PAPP2\":\"" + String(val_P[2]) + "\",\"PAPP3\":\"" + val_P[3] + "\",\"Time\":\"";//\"IINST\":\"" + String(val_I[0]) + "\"}";
            done = true;
            isADCO = false;
            flag_P[1] = flag_P[2] = flag_P[3] = false;
        }
    }
}

void setDone(bool d)
{
    done = d;
}

bool is_Done()
{
    return done;
}

bool getParity(unsigned int n)
{
    return __builtin_parity(n);
}

bool checksum(String data,char chk)
{
    char c[data.length()+1];
    int check = 0;
    strcpy(c , data.c_str());
    for(int i = 0 ; i<data.length();i++)
    {
        int N = c[i];
        if(getParity(N))
            check+=c[i]&0x7f;
        else
            check+=(c[i]&0x7f) | 0x80;
    }
    // Serial.println("---------"); // Serial.println("check= "+ String(check) );
    check = check & 0x3f;
    // Serial.println("check= "+ String(check) );
    check = check + 0x20;
    // Serial.println("check= "+ String(check)); // Serial.print("check ascii= "); // Serial.println((char) check); // Serial.print("rcv check: "); // Serial.println(chk);
    debugln((char)check);
    if (check == int(chk))
        return true;
    return false;
}

String getJson()
{
    return json_frame;
}