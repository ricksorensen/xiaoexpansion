#pragma once
typedef struct fnote {
  float f;
  int duration;
} freq_note;

typedef struct nnote {
  char name;
  int duration;
} named_note;

extern void doBuzzer(unsigned char buzzpin);
