#ifndef SETTING_H
#define SETTING_H

#include "mat33.h"


//definition of global variables which are set by "setting.ini"//

extern int g_orientation_sensor_enable;

extern int g_gpu_DX11;
extern int g_vsync_mode;
extern TCHAR g_dxgi_format_str[256];


extern int g_window_width;
extern int g_window_height;
//extern int g_multiview_mode;


extern double g_epsAtomSize;
extern double g_epsBondSize;
extern double g_epsBoxLineSize;




extern double g_bond_cutoff;
extern int g_visual_history_frame;

extern int g_stl_grid_cutoff_edge_z;
extern double g_stl_grid_width;
extern double g_stl_scale;
extern int g_stl_atompoly;
extern double g_stl_atomradius;
extern int g_stl_bondpoly;


extern double g_visual_trajectory_width;
extern float g_visual_trajectory_color[4];


extern int g_LargeBMP_mag;


extern int g_periodic;
extern mat33f g_default_boxaxis;
extern int g_trajectory_z_reverse;


extern double VIEW_ANGLE;

extern double g_movie_delta_angle_deg;

extern int g_DefaultBMP_mag;
extern TCHAR g_DefaultOutputPath[1024];

extern double g_color_hight_center;
extern double g_color_hight_range;

#endif //!SETTING_H