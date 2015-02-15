//========================================================
// File Name: ir_controller
// URL      : http://make.bcde.jp
//========================================================
#include "types.h"

//========================================================
// Config
//========================================================

#define SERIAL_BPS             (57600)   /* Serial bps */
#define IR_IN                  (8)       /* Input      */
#define IR_OUT                 (13)      /* Output     */

#define TIMEOUT_RECV_NOSIGNAL  (50000)
#define TIMEOUT_RECV           (5000000)
#define TIMEOUT_SEND           (2000000)

//========================================================
// Define
//========================================================
#define STATE_NONE             (-1)
#define STATE_OK               (0)
#define STATE_TIMEOUT          (1)
#define STATE_OVERFLOW         (2)
#define DATA_SIZE              (800)

//========================================================
// Program
//========================================================
u2 data[DATA_SIZE];

void sendSignal(){
  u1 x;
  s1 state = STATE_NONE;
  u2 time = 0;
  u4 tmp = 0;
  u4 index = 0;
  u4 count = 0;
  u4 us = 0;
  
  us = micros();
  
  while(state == STATE_NONE){
    if(Serial.available() == 0){
      if((micros() - us) > TIMEOUT_SEND){
        state = STATE_TIMEOUT;
        break;
      }
    } else {
      x = Serial.read();
      if(x>='0' && x<='9'){
        /* 数字を受信した場合 */
        tmp *= 10;
        tmp += x - '0';
      } else {
        /* 数字以外を受信した場合 */
        if((tmp == 0) && (index == 0)){
          /* 最初の一文字目は読み飛ばす */
        } else {
          data[index] = (u2)tmp;
          if(tmp == 0){
            state = STATE_OK;
            break;
          } else if(index >= DATA_SIZE){
            state = STATE_OVERFLOW;
            break;
          }
          index++;
        }
        tmp = 0;
      }
    }
  }

  if(state == STATE_OK){
    for(count = 0; count < index; count++){
      time = data[count];
      us = micros();
      do {
        digitalWrite(IR_OUT, !(count&1));
        delayMicroseconds(8);
        digitalWrite(IR_OUT, 0);
        delayMicroseconds(7);
      }while(s4(us + time - micros()) > 0);
    }
    Serial.println("OK");
  } else {
    Serial.print("NG:");
    Serial.println(state);
  }
}

void recvSignal(){
  
  u1 pre_value = HIGH;
  u1 now_value = HIGH;
  u1 wait_flag = TRUE;
  s1 state = STATE_NONE;
  u4 pre_us = micros();
  u4 now_us = 0;
  u4 index = 0;
  u4 i = 0;
  
  while(state == STATE_NONE){
    now_value = digitalRead(IR_IN);
    if(pre_value != now_value){
      now_us = micros();
      if(!wait_flag){
        data[index++] = now_us - pre_us;
      }
      wait_flag = FALSE;
      pre_value = now_value;
      pre_us = now_us;
    }
    
    if(wait_flag){
        if((micros() - pre_us) > TIMEOUT_RECV){
          state = STATE_TIMEOUT;
          break;
        }
      } else {
        if((micros() - pre_us) > TIMEOUT_RECV_NOSIGNAL){
          state = STATE_OK;
          break;
        }
      }
  }
  
  if(state == STATE_OK){
    Serial.print("s,");
    for(i = 0; i<index; i++){
      Serial.print(data[i]);
      Serial.print(',');
    }
    Serial.println("0,");
  } else {
    Serial.println("NG");
  }
}

void setup(){
  Serial.begin(SERIAL_BPS);
  pinMode(IR_IN, INPUT);
  pinMode(IR_OUT, OUTPUT);
}

void loop(){
  u1 x;
  if(Serial.available()){
    x=Serial.read();
    switch(x){
      case 's':
        sendSignal();
        break;
      case 'r':
        recvSignal();
        break;
      default:
        break;
    }
  }
}
        
