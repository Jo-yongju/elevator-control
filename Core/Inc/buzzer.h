#ifndef BUZZER_H
#define BUZZER_H

#include "main.h"

void Buzzer_Init(void);
void Buzzer_PlayDoorOpen(void);
void Buzzer_PlayDoorClose(void);
void Buzzer_Stop(void);
void Buzzer_Update(void);
uint8_t Buzzer_IsBusy(void);

#endif /* BUZZER_H */
