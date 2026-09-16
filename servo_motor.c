#define FCY 3685000UL
#include <xc.h>
#include <libpic30.h>
#include <stdint.h>


// --- GÜNCEL P?N TANIMLAMALARI ---
#define CLK_PIN         LATEbits.LATE2    // Pin 21
#define CW_CCW_PIN      LATAbits.LATA5    // Pin 22
#define ENABLE_PIN      LATCbits.LATC15   // Pin 18
#define RESET_PIN       LATCbits.LATC12   // Pin 19

#define M1_PIN          LATHbits.LATH12   // Pin 7
#define M2_PIN          LATAbits.LATA15   // Pin 8
#define M3_PIN          LATAbits.LATA14   // Pin 9
#define LATCH_PIN       LATHbits.LATH13   // Pin 4

#define ALERT_PIN       PORTCbits.RC14    // Pin 1 (Input - Hata sinyali)
#define TQ_PIN          LATCbits.LATC13   // Pin 3 (Output - Tork/Is? kontrol)
#define MO_PIN          PORTAbits.RA4     // Pin 25 (Input - Monitör sinyali)
volatile uint32_t current_step = 0;
// 1 tam tur = 5400 ad?m. Toggle i?lemi için x2 pulse üretmeliyiz.
volatile uint32_t target_toggles = 5400 * 2; 
volatile uint8_t motor_running = 0;
// --- BA?LATMA FONKS?YONU ---
void System_Init(void) {
    // Ç?k?? (Output) Pinleri
    TRISEbits.TRISE2 = 0;   
    TRISAbits.TRISA5 = 0;   
    TRISCbits.TRISC15 = 0;  
    TRISCbits.TRISC12 = 0;  
    TRISHbits.TRISH12 = 0;  
    TRISAbits.TRISA15 = 0;  
    TRISAbits.TRISA14 = 0;  
    TRISHbits.TRISH13 = 0;  
    TRISCbits.TRISC13 = 0;  // TQ (Is? kontrolü için eklendi)

    // Giri? (Input) Pinleri
    TRISCbits.TRISC14 = 1;  // ALERT
    TRISAbits.TRISA4 = 1;   // MO

    // Ba?lang?ç Durumlar?
    ENABLE_PIN = 1;  
    RESET_PIN = 1;   
    LATCH_PIN = 0;   
    
    // Tork Kontrolü (Motor dururken ak?m?/?s?nmay? azaltmak için)
    TQ_PIN = 1; 

    // Çözünürlük (1/1 Full Step -> L L H)
    M1_PIN = 0;
    M2_PIN = 0;
    M3_PIN = 1;
    
    CW_CCW_PIN = 1;  
    CLK_PIN = 0;     
}
void Timer1_Init(void) {
    T1CONbits.TON = 0;      // Ayar yaparken Timer'? kapat
    T1CONbits.TCKPS = 0b01; // Prescaler 1:8 (Timer Clock = FCY / 8 = 460.625 kHz)
    
    // PR1 = 460 oldu?unda ~1000 Hz kesme olu?ur. 
    // Her iki kesmede 1 ad?m at?l?r, yani saniyede 500 ad?m h?z? elde edilir.
    // Motoru h?zland?rmak için PR1 de?erini dü?ür, yava?latmak için art?r.
    PR1 = 460;              
    
    IPC0bits.T1IP = 4;      // Kesme önceli?i
    IFS0bits.T1IF = 0;      // Kesme bayra??n? temizle
    IEC0bits.T1IE = 1;      // Timer1 kesmesini aktif et
}

// --- TIMER KESMES? (ISI YÖNET?M? EKLENM?? HAL?) ---
void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; 
    
    if (motor_running) {
        TQ_PIN = 0; // Hareket halindeyken tam tork sa?la
        
        if (current_step < target_toggles) {
            CLK_PIN = ~CLK_PIN; 
            current_step++;
        } else {
            motor_running = 0; 
            T1CONbits.TON = 0; 
            
            TQ_PIN = 1; // Hedefe ula?t???nda torku dü?ürerek motorun/sürücünün ?s?nmas?n? engelle
        }
    }
}
int main(void) {
    System_Init();
    Timer1_Init();
    
    // Hedeflenen dönü? için motoru ba?lat
    current_step = 0;
    motor_running = 1;
    T1CONbits.TON = 1; // Timer'? ba?lat ve sinyal üretimine geç
    
    while (1) {
        // Motor arka planda Timer Kesmesi ile dönerken 
        // dsPIC burada serbest kal?r. 
        // ?leride enkoder okuma i?lemlerini bu while(1) döngüsüne ekleyeceksin.
    }

    return 0;
}