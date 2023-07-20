
#include "outeps.h"

void outEPS_header(FILE* fp, int bx1, int by1, int bx2, int by2){
	_ftprintf(fp,_T("%%!PS-Adobe-3.0 EPSF-3.0\n"));
	_ftprintf(fp,_T("%%%%Creator: outEPS ver.0\n"));
	//_ftprintf(fp,_T("%%CreationDate: 2007/07/30 14:32:01\n"));
	_ftprintf(fp,_T("%%%%Title: gh1\n"));
	_ftprintf(fp,_T("%%%%DocumentFonts: (atend)\n"));
	_ftprintf(fp,_T("%%%%BoundingBox: %d %d %d %d\n"), bx1, by1, bx2, by2);
	_ftprintf(fp,_T("%%%%Orientation: Portrait\n"));
	_ftprintf(fp,_T("%%%%EndComments\n"));
	_ftprintf(fp,_T("/mydict 120 dict def\n"));
	_ftprintf(fp,_T("mydict begin\n"));
	_ftprintf(fp,_T("\n"));
	_ftprintf(fp,_T("gsave\n"));
}

void outEPS_footer(FILE* fp){
	outEPS_stroke(fp);
	_ftprintf(fp,_T("grestore\n"));
	_ftprintf(fp,_T("end\n"));
	_ftprintf(fp,_T("showpage\n"));
	_ftprintf(fp,_T("%%%%Trailer\n"));
	_ftprintf(fp,_T("%%%%EOF\n"));
}


void outEPS_pathCross(FILE* fp,double x, double y, double w){
	outEPS_newpath(fp);
	outEPS_moveto(fp, (x - w/2),(y + w/2));
	outEPS_rlineto(fp, w, -w);
	outEPS_moveto(fp, (x + w/2),(y + w/2));
	outEPS_rlineto(fp, -w, -w);
	outEPS_closepath(fp);
}

void outEPS_drawCross(FILE* fp,double x, double y, double w){
	outEPS_pathCross(fp,x,y,w);
	outEPS_stroke(fp);
}

void outEPS_pathCircle(FILE* fp,double x, double y, double r){
	outEPS_newpath(fp);
	outEPS_arc(fp, x,y,r,0, 360);
	outEPS_closepath(fp);
}

void outEPS_drawCircle(FILE* fp,double x, double y, double r){
	outEPS_pathCircle(fp,x,y,r);
	outEPS_stroke(fp);

}

void outEPS_drawCircle(FILE* fp,double x, double y, double r, double red, double green, double blue){
	
	outEPS_gsave(fp);
	outEPS_pathCircle(fp,x,y,r);
	outEPS_setrgbcolor(fp,red,green,blue);
	outEPS_fill(fp);

	outEPS_pathCircle(fp,x,y,r);
	outEPS_stroke(fp);
	outEPS_grestore(fp);

}


void outEPS_pathSquare(FILE* fp,double x, double y, double w){
	outEPS_newpath(fp);
	outEPS_moveto(fp, x - w/2, y + w/2);
	outEPS_rlineto(fp, w, 0.0);
	outEPS_rlineto(fp, 0.0, -w);
	outEPS_rlineto(fp, -w, 0.0);
	outEPS_closepath(fp);
}

void outEPS_drawSquare(FILE* fp,double x, double y, double w){
	outEPS_pathSquare(fp,x,y,w);
	outEPS_stroke(fp);
}


void outEPS_drawSquare(FILE* fp,double x, double y, double w, double red, double green, double blue){
	
	outEPS_gsave(fp);
	
	outEPS_pathSquare(fp,x,y,w);
	outEPS_setrgbcolor(fp,red,green,blue);
	outEPS_fill(fp);

	
	outEPS_pathSquare(fp,x,y,w);
	outEPS_stroke(fp);
	
	outEPS_grestore(fp);
}

void outEPS_pathTriangle(FILE* fp,double x, double y, double w){
	double a;
	a = (double)(w / sqrt(3.0));
	outEPS_newpath(fp);
	outEPS_moveto(fp, x, (y + a));
	outEPS_rlineto(fp, -w/2.0f, -a*1.5f);
	outEPS_rlineto(fp, w, 0.0);
	outEPS_closepath(fp);
}

void outEPS_drawTriangle(FILE* fp,double x, double y, double w){
	outEPS_pathTriangle(fp,x,y,w);
	outEPS_stroke(fp);
}

void outEPS_drawTriangle(FILE* fp,double x, double y, double w, double red, double green, double blue){
	
	outEPS_gsave(fp);

	outEPS_pathTriangle(fp,x,y,w);
	outEPS_setrgbcolor(fp,red,green,blue);
	outEPS_fill(fp);

	
	outEPS_pathTriangle(fp,x,y,w);
	outEPS_stroke(fp);

	outEPS_grestore(fp);
}

void outEPS_newpath(FILE* fp){
	_ftprintf(fp,_T("newpath\n"));
}

void outEPS_closepath(FILE* fp){
	_ftprintf(fp,_T("closepath\n"));
}

void outEPS_stroke(FILE* fp){
	_ftprintf(fp,_T("stroke\n"));
}

void outEPS_fill(FILE* fp){
	_ftprintf(fp,_T("fill\n"));
}

void outEPS_gsave(FILE* fp){
	_ftprintf(fp,_T("gsave\n"));
}

void outEPS_grestore(FILE* fp){
	_ftprintf(fp,_T("grestore\n"));
}

void outEPS_translate(FILE* fp,double x, double y){
	_ftprintf(fp,_T("%8.3lf %8.3lf translate\n"), x, y);
}

void outEPS_moveto(FILE* fp,double x, double y){
	_ftprintf(fp,_T("%8.3lf %8.3lf moveto\n"), x, y);
}

void outEPS_rmoveto(FILE* fp,double x, double y){
	_ftprintf(fp,_T("%8.3lf %8.3lf rmoveto\n"), x, y);
}

void outEPS_lineto(FILE* fp,double x, double y){
	_ftprintf(fp,_T("%8.3lf %8.3lf lineto\n"), x, y);
}

void outEPS_rlineto(FILE* fp,double x, double y){
	_ftprintf(fp,_T("%8.3lf %8.3lf rlineto\n"), x, y);
}

void outEPS_arc(FILE* fp,double x, double y, double r, double start_agl,double end_agl){
	_ftprintf(fp,_T("%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf arc\n"),x,y,r,start_agl,end_agl);
}

void outEPS_setrgbcolor(FILE* fp, double red, double green, double blue){
	_ftprintf(fp,_T("%8.3lf %8.3lf %8.3lf setrgbcolor\n"), red, green, blue);
}

void outEPS_currentrgbcolor(FILE* fp){
	_ftprintf(fp,_T("currentrgbcolor\n"));
}

void outEPS_setlinewidth(FILE* fp, double width){
	_ftprintf(fp,_T("%8.3lf setlinewidth\n"), width);
}

void outEPS_drawString(FILE* fp, double x, double y, const char* str, int fontsize, const char* fontname){
	outEPS_moveto(fp,x,y);
	_ftprintf(fp,_T("/%s findfont %d scalefont setfont\n"), fontname, fontsize);
	_ftprintf(fp,_T("(%s) show\n"),str);
}