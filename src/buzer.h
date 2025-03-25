#ifndef __BUZER_H__
#define __BUZER_H__


#define BUZER_ON()          digitalWrite(BUZER_PIN, LOW)
#define BUZER_OFF()         digitalWrite(BUZER_PIN, HIGH)
#define BEEP(DELAY_MS)      do{BUZER_ON();delay(DELAY_MS);BUZER_OFF();}while(0)
#define BEEPS(COUNT,BEEP_DELAY_MS,PAUSE_DELAY_MS) \
    do{ \
        for(uint8_t bcnt = 0; bcnt < COUNT; bcnt++) { \
            if(bcnt) delay(PAUSE_DELAY_MS); \
            BEEP(BEEP_DELAY_MS); \
        } \
    }while(0)
    
#endif // __BUZER_H__