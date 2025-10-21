/***************************************************************************
  DPRINTF 1.10                                           written in C++
***************************************************************************/

#ifndef __DPRINTF_H__
#define __DPRINTF_H__


void dprintf(const char*, ...);
void dprintf(unsigned int Level, const char*, ...);
void dprintf(int X, int Y, const char*, ...);
void dprintf(unsigned int Level, int X, int Y, const char*, ...);
void dgotoxy(int X, int Y);
void dgotoxy(unsigned int Level, int X, int Y);
void dclrscr(void);
void dclrscr(unsigned int Level);

#endif
