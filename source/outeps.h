#ifndef __outeps_h__
#define __outeps_h__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <tchar.h>


void outEPS_header(FILE* fp, int bx1, int by1, int bx2, int by2);
void outEPS_footer(FILE* fp);
void outEPS_pathCross(FILE* fp,double x, double y, double w);
void outEPS_drawCross(FILE* fp,double x, double y, double w);
void outEPS_pathCircle(FILE* fp,double x, double y, double r);
void outEPS_drawCircle(FILE* fp,double x, double y, double r);
void outEPS_drawCircle(FILE* fp,double x, double y, double r, double red, double green, double blue);
void outEPS_pathSquare(FILE* fp,double x, double y, double w);
void outEPS_drawSquare(FILE* fp,double x, double y, double w);
void outEPS_drawSquare(FILE* fp,double x, double y, double w, double red, double green, double blue);
void outEPS_pathTriangle(FILE* fp,double x, double y, double w);
void outEPS_drawTriangle(FILE* fp,double x, double y, double w);
void outEPS_drawTriangle(FILE* fp,double x, double y, double w, double red, double green, double blue);



void outEPS_newpath(FILE* fp);
void outEPS_closepath(FILE* fp);
void outEPS_stroke(FILE* fp);
void outEPS_fill(FILE* fp);
void outEPS_gsave(FILE* fp);
void outEPS_grestore(FILE* fp);

void outEPS_translate(FILE* fp,double x, double y);
void outEPS_moveto(FILE* fp,double x, double y);
void outEPS_rmoveto(FILE* fp,double x, double y);
void outEPS_lineto(FILE* fp,double x, double y);
void outEPS_rlineto(FILE* fp,double x, double y);
void outEPS_arc(FILE* fp,double x, double y, double r, double start_agl,double end_agl);

void outEPS_setrgbcolor(FILE* fp, double red, double green, double blue);
void outEPS_currentrgbcolor(FILE* fp);

void outEPS_setlinewidth(FILE* fp, double width);


void outEPS_drawString(FILE* fp, double x, double y, const char* str, int fontsize, const char* fontname);

#endif    // __outeps_h__