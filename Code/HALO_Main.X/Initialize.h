/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef INITIALIZE_H
#define	INITIALIZE_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define ON 1
#define OFF 0

#define INPUT 1
#define OUTPUT 0

/* * * * * * * * * * * * On-Board LED Definitions * * * * * * * * * * * */
#define LED1 LATDbits.LATD8
#define LED2 LATBbits.LATB5
#define LED3 LATBbits.LATB6
#define LED4 LATBbits.LATB9
#define LED5 LATDbits.LATD5
#define LED6 LATDbits.LATD6
#define LED7 LATFbits.LATF0
#define LED8 LATFbits.LATF1

#define pinModeLED1 TRISDbits.TRISD8
#define pinModeLED2 TRISBbits.TRISB5
#define pinModeLED3 TRISBbits.TRISB6
#define pinModeLED4 TRISBbits.TRISB9
#define pinModeLED5 TRISDbits.TRISD5
#define pinModeLED6 TRISDbits.TRISD6
#define pinModeLED7 TRISFbits.TRISF0
#define pinModeLED8 TRISFbits.TRISF1
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

//bool Initialize();
void Start_Initialization();
//switches the clock from the FRC to the external HS oscillator
void oscillator(void);
void timerOne(void);
void timerTwo(void);

void init_SPI();

#endif	/* XC_HEADER_TEMPLATE_H */

