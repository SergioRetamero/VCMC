// Copyright 2019 Sergio Retamero.
//
// Author: Sergio Retamero (sergio.retamero@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
#include "Definitions.h"

/** @addtogroup SysEx
 *  @{
 */

/**
 *  \file SysExConfigPort.ino
 *  \brief SysEx Send/Receive support for configuration classes
 */  


void GlobalCfg::SaveCfgSysEx (uint8_t par)
{
    byte bType = (DUMPCHANNEL<<4);
    byte bSlot = 0; // No port (global config) 0= Dump full config

    //byte pSource[49];
    uint16_t uLengthTotal = sizeof(GlobalCfg);
    uint16_t uLengthUserNames = sizeof(UserNames);
    uint16_t uLengthSource = uLengthTotal-uLengthUserNames;
    if(uLengthSource>49)
        return; // too much data
    #ifdef PRINTDEBUG
    Serial.println("Sending Globl Config");
    #endif
    // Send parameters
    FlashAccess->SaveCfgSingleSysEx((byte *)this, uLengthSource, bSlot, bType);
    // Send user strings. Limit SysEx to 64 bytes:
    byte offs = uLengthSource;
    uint16_t leng = (SIZEPORTNAMES+1)*6;

    // Port = 1 send first 6 strings
    FlashAccess->SaveCfgSingleSysEx(((byte *)this)+offs, leng, bSlot+(1<<4), bType);
    // Port = 2 next  6 strings
    offs = offs+leng;
    FlashAccess->SaveCfgSingleSysEx(((byte *)this)+offs, leng, bSlot+(2<<4), bType);
    // Port = 3 next  3 strings
    offs = offs+leng;
    leng = (SIZEPORTNAMES+1)*3;
    FlashAccess->SaveCfgSingleSysEx(((byte *)this)+offs, leng, bSlot+(3<<4), bType);
    
}

bool GlobalCfg::ReadCfgSysEx(byte* DecodedData,unsigned int  decLen, uint8_t bPort)
{
    #ifdef PRINTDEBUG
    Serial.print("Received Global Config: ");
    Serial.print(bPort);
    Serial.print(" / ");
    if( bPort)
        Serial.println((char *)DecodedData);
    else Serial.println(decLen);
    #endif

    if( bPort){
        uint8_t index = (bPort - 1) * 6;
        uint8_t numstr = (bPort == 3) ? 3 : 6;
        for (int i = 0; i < numstr;i++){
            memcpy(UserNames[index+i], DecodedData + i * 7, 7);
            #ifdef PRINTDEBUG
            Serial.print(index+i+1);
            Serial.print(" UserName: ");
            Serial.println(UserNames[index+i]);
            #endif
        }
    } else {
        /* if( decLen!=sizeof(GlobalCfg))
            return false; */
        memcpy(this, DecodedData, decLen);
    }
    #ifdef PRINTDEBUG
    Serial.print("Received Global Config: ");
    Serial.println(decLen);
    #endif    
    // TODO Accept data somehow¿?
    return true;
}


const uint8_t DELAYTOWRITE = 20;
void InputControl::SaveCfgSysEx (uint8_t par)
{
    // Send Control Config
    Config.SaveCfgSysEx(0, ControlNumber + 1);
    delay(DELAYTOWRITE);

    // Send  CV
    CVPort.PortCfg.SaveCfgSysEx( 0, ControlNumber + 1, CVSLOT);
    delay(DELAYTOWRITE);
 
    // Send  Fader
    Slider.PortCfg.SaveCfgSysEx( 0, ControlNumber + 1, FADERSLOT);
    delay(DELAYTOWRITE);

    // Send  Gate
    GateBut.PortCfg.SaveCfgSysEx( 0, ControlNumber + 1, GATESLOT);
    delay(DELAYTOWRITE);
}

bool InputControl::ReadCfgSysEx(VCMCSysExPacket *SysExPacket, byte* DecodedData,unsigned int  decLen)
{
  byte bPort = (SysExPacket->Slot>>4);
  //byte bParam = SysExPacket->Slot & 0xf;

    switch(bPort)
    {
        case GENSLOT:
            return Config.ReadCfgSysEx(DecodedData, decLen);
        case CVSLOT:
            return CVPort.PortCfg.ReadCfgSysEx(DecodedData, decLen);
        case FADERSLOT:
            return Slider.PortCfg.ReadCfgSysEx(DecodedData, decLen);
        case GATESLOT:
            return GateBut.PortCfg.ReadCfgSysEx(DecodedData, decLen);
    }
    // TODO Accept data somehow¿?
    return true;
}

void InputCtrlCfg::SaveCfgSysEx (uint8_t par,uint8_t chan)
{
    byte bType = (DUMPCHANNEL<<4)+chan;
    byte bSlot = GENSLOT<<4; // No port (global config) 0= Dump full config

    //byte pSource[49];
    uint16_t uLengthSource = sizeof(InputCtrlCfg);
    if(uLengthSource>49)
        return; // too much data
    #ifdef PRINTDEBUG
    Serial.print("Sending InputCtrl: ");
    Serial.println(chan);
    #endif
    FlashAccess->SaveCfgSingleSysEx((byte *)this, uLengthSource, bSlot, bType);
}


bool InputCtrlCfg::ReadCfgSysEx(byte* DecodedData,unsigned int  decLen)
{
    if( decLen!=sizeof(InputCtrlCfg))
        return false;
    memcpy(this, DecodedData, decLen);

    #ifdef PRINTDEBUG
    Serial.print("Received Input Control Config: ");
    Serial.println(decLen);
    #endif    
    // TODO Accept data somehow¿?
    return true;
}


void AnInputPortCfg::SaveCfgSysEx (uint8_t par, uint8_t chan, uint8_t slot)
{
    byte bType = (DUMPCHANNEL<<4)+chan;
    byte bSlot = slot<<4; // port (slot) 0= Dump full config

    //byte pSource[49];
    uint16_t uLengthSource = sizeof(AnInputPortCfg);
    if(uLengthSource>49)
        return; // too much data
    #ifdef PRINTDEBUG
    Serial.print("Sending Analog Port: ");
    Serial.print(chan);
    Serial.print("/");
    Serial.print(slot);
    Serial.print("/");
    Serial.println(uLengthSource);
    #endif
    FlashAccess->SaveCfgSingleSysEx((byte *)this, uLengthSource, bSlot, bType);

}


bool AnInputPortCfg::ReadCfgSysEx(byte* DecodedData,unsigned int  decLen)
{
    if( decLen!=sizeof(AnInputPortCfg)){
        #ifdef PRINTDEBUG
        Serial.println("Received incorrect Analog Port Config:");
        Serial.printf("Expected: %d Received: %d\n", sizeof(AnInputPortCfg), decLen);
        #endif    
        return false;
        }
    memcpy(this, DecodedData, decLen);

    #ifdef PRINTDEBUG
    Serial.print("Received Analog Port Config: ");
    Serial.println(decLen);
    #endif    
    // TODO Accept data somehow¿?
    return true;
}


void DigPortCfg::SaveCfgSysEx (uint8_t par, uint8_t chan, uint8_t slot)
{
    byte bType = (DUMPCHANNEL<<4)+chan;
    byte bSlot = slot<<4; // port (slot) 0= Dump full config

    //byte pSource[49];
    uint16_t uLengthSource = sizeof(DigPortCfg);
    if(uLengthSource>49)
        return; // too much data
    #ifdef PRINTDEBUG
    Serial.print("Sending Digital Port: ");
    Serial.print(chan);
    Serial.print("/");
    Serial.println(slot);
    #endif
    FlashAccess->SaveCfgSingleSysEx((byte *)this, uLengthSource, bSlot, bType);
}

bool DigPortCfg::ReadCfgSysEx(byte* DecodedData,unsigned int  decLen)
{
    if( decLen!=sizeof(DigPortCfg))
        return false;
    memcpy(this, DecodedData, decLen);

    #ifdef PRINTDEBUG
    Serial.print("Received Digital Port Config: ");
    Serial.println(decLen);
    #endif    
    // TODO Accept data somehow¿?
    return true;
}

/** @} */
