 /* Morse Code Message Pendant - see http://www.technoblogy.com/show?3TAC

   David Johnson-Davies - www.technoblogy.com - 14th February 2022
   ATtiny10 @ 1MHz (internal oscillator)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <avr/sleep.h>
#include <avr/interrupt.h>

// Constants

const int LED = 2;                        // PB2
const unsigned int Dot = 200;             // Dot duration in milliseconds

uint8_t Power __attribute__ ((section (".noinit")));

const char PROGMEM Message[] = "You're the one I want to share my life with. ";

const uint8_t PROGMEM Chars[48] = {
//A           B           C           D           E           F
  0b01100000, 0b10001000, 0b10101000, 0b10010000, 0b01000000, 0b00101000,
//G           H           I           J           K           L
  0b11010000, 0b00001000, 0b00100000, 0b01111000, 0b10110000, 0b01001000,
//M           N           O           P           Q           R
  0b11100000, 0b10100000, 0b11110000, 0b01101000, 0b11011000, 0b01010000,
//S           T           U           V           W           X
  0b00010000, 0b11000000, 0b00110000, 0b00011000, 0b01110000, 0b10011000,
//Y           Z           0           1           2           3
  0b10111000, 0b11001000, 0b11111100, 0b01111100, 0b00111100, 0b00011100,
//4           5           6           7           8           9
  0b00001100, 0b00000100, 0b10000100, 0b11000100, 0b11100100, 0b11110100,
//+           ,           -           .           /           ?
  0b01010110, 0b11001110, 0b10000110, 0b01010110, 0b10010100, 0b00110010,
//&           '           (           )           !           "            
  0b10101110, 0b01000100, 0b10110100, 0b10110110, 0b10101110, 0b01001010,
};

// Timer **********************************************

// Wait for n dot units
void Wait (unsigned int n) {
  GTCCR = 1<<TSM | 1<<PSR;                // Keep the counter reset
  OCR0A = Dot * n;                        // Define compare value
  TIFR0 = 1<<OCF0A;                       // Clear flag
  GTCCR = 0;                              // Start counter
  while ((TIFR0 & 1<<OCF0A) == 0);        // Wait for compare match
}

// Generate morse **********************************************

void DotDash (bool dash) {
  PORTB = PORTB | 1<<LED;                 // LED on
  if (dash) Wait(3); else Wait(1);
  PORTB = PORTB & ~(1<<LED);              // LED off
  Wait(1);
}

void Letter (char letter) {
  uint8_t index;
  letter = letter | 0x20;                 // Convert letters to lower case
  if (letter == ' ') { Wait(4); return; }
  else if (letter >= 'a' && letter <= 'z') index = letter - 'a';
  else if (letter >= '0' && letter <= '9') index = letter - '0' + 26;
  else if (letter >= '+' && letter <= '/') index = letter - '+' + 36;
  else if (letter == '?') index = 41;
  else if (letter >= '&' && letter <= ')') index = letter - '&' + 42;
  else if (letter >= '!' && letter <= '"') index = letter - '!' + 46;
  else return;
  uint8_t code = Chars[index];
  while (code != 0x80) {
    DotDash(code & 0x80);
    code = code<<1;
  }
  Wait(2);
}

// Setup **********************************************

void setup () {
  DDRB = 0x07;                             // All outputs
  // Set up Timer/Counter for 1 msec per count
  TCCR0A = 0<<WGM00;                       // CTC mode
  TCCR0B = 1<<WGM02 | 5<<CS00;             // CTC mode, / 1024
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  Wait(6);
  Power = ~Power & 1;
}

void loop () {
  int p = 0;
  while (Power) {
    char c = Message[p++];
    if (c == 0) break;
    Letter(c);
  }
  PORTB = 0;                               // Turn off LED
  sleep_enable();
  sleep_cpu();
}
