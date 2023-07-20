#ifndef RenderGL15_H
#define RenderGL15_H

#include "filelist.h"
#include "vec3.h"
#include "mat33.h"

struct RenderingProperty;

//void RenderingGL15(VP_FileList& files, int num_files, int wnd_w, int wnd_h, CameraGL* cam);
void resize(int w, int h);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void special_key(int key, int x, int y);
void motion(int x, int y);
void idle(void);
void timer(int value);
void InitGL15(void);
//void termin(void);
void movtimer(int value);

void DrawCubeGL15(mat33f* axis, vec3f* org, const float* bg_color);

void OutputBMfromGLMag(std::vector<LawData>& data_list, const TCHAR *fName, int width, int height, int magnify, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property);
void SetProjectionRangeGL(int projection_mode, int image_w, int image_h, int range_x, int range_y, int range_w, int range_h, double distance);
void RenderingGL15_2(std::vector<LawData>& data_list, int screen_w, int screen_h, int range_x, int range_y, int range_w, int range_h, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property);
void OutputRaytracing(LawData& data, const TCHAR *fName, int width, int height, int magnify, const double* view_matrix, const double focus_distance);


class UIPanel;
void DrawUIPanelGL15(int screen_w, int screen_h, const UIPanel& uipanel);


#endif    // RenderGL15_H