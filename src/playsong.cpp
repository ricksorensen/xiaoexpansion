#include <Arduino.h>
#include "playsong.h"

//char notes[] = "GGAGcB GGAGdc GGxecBA yyecdc";
named_note notes[] = {
  { 'G', 2},
  { 'G', 2},
  { 'A', 8},
  { 'G', 8},
  { 'c', 8},
  { 'B', 16},
  { ' ', 1},
  { 'G',2},
  {'G',2},
  {'A',8},
  {'G',8},
  {'d',8},
  {'c',16},
  {' ',1},
  {'G',2},
  {'G',2},
  {'x',8},
  {'e',8},
  {'c',8},
  {'B',8},
  {'A',16},
  {' ',1},
  {'y',2},
  {'y',2},
  {'e',8},
  {'c',8},
  {'d',8},
  {'c',16}
};
int length = sizeof(notes)/sizeof(named_note);

//int beats[] = { 2, 2, 8, 8, 8, 16, 1, 2, 2, 8, 8, 8, 8, 16, 1, 2, 2, 8, 8, 8, 16 };
int tempo = 150;
void playTone(int tone, int duration, unsigned char buzzpin) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(buzzpin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(buzzpin, LOW);
    delayMicroseconds(tone);
  }
}
 
void playNote(char note, int duration, unsigned char buzzpin) {
  char names[] = {'C', 'D', 'E', 'F', 'G', 'A', 'B',
                  'c', 'd', 'e', 'f', 'g', 'a', 'b',
                  'x', 'y'
  };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014,
                  956,  834,  765,  593,  468,  346,  224,
                  655 , 715
  };
  int SPEE = 5;
 
  // play the tone corresponding to the note name
 
  for (int i = 0; i < 16; i++) {
    if (names[i] == note) {
      int newduration = duration / SPEE;
      playTone(tones[i], newduration, buzzpin);
    }
  }
}

void doBuzzer(unsigned char buzzpin) {
  for (int i = 0; i < length; i++) {
    if (notes[i].name == ' ') {
      delay(notes[i].duration * tempo); // rest
    } else {
      playNote(notes[i].name, notes[i].duration * tempo, buzzpin);
    }
    // pause between notes
    delay(tempo);
  }
}
