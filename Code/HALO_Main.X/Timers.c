/*
 * File:   Timers.c
 * Author: Hunter Hykes
 *
 * Created on February 1, 2021, 12:50 PM
 */

#include <xc.h>
#include <stdlib.h>
#include "Timers.h"
#include "Initialize.h"

// *****************************************
// ******** TIMERS.C file code *************
// *****************************************

unsigned long globalTime;

unsigned long millis(void)
{
    return globalTime;
}

bool timerDone(timers_t * t)
{
    if(abs(millis() - t->lastMillis) > t->timerInterval)
    {
        t->lastMillis = millis();
        return true;
    }
    else
    {
        return false;
    }
}

void setTimerInterval(timers_t * t, unsigned long interval)
{
    if(t->timerInterval != interval)
        t->timerInterval = interval;
}

void resetTimer(timers_t * t)
{
    t->lastMillis = millis();
}

// Call this function in your timer interrupt that fires at 1ms
void globalTimerTracker()
{
    globalTime++;
}

timers_t time;
void delay(int interval)
{
    time.timerInterval = interval;
    time.lastMillis = millis();
    while(!timerDone(&time));
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    globalTimerTracker();

    IFS0bits.T1IF = 0; // clear interrupt flag
}

//*****************************************
//**********  END OF C file  **************
//*****************************************
