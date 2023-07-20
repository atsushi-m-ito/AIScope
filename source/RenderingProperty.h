#pragma once

#include "CrossSection.h"

struct RenderingProperty
{
	int projection_mode;		//0: 正射影, 1: 投影射影//
	int box_draw;				//0: ボックス枠を表示しない, 1:する//
	int multiview_mode;			//0: overlap, 1:arranged//

	int atom_draw;
	int atom_color;
	int atom_trajectory;
	int atom_poly;
	int bond_draw;
	int bond_poly;
	double atom_radius;
	float bg_color[4];
	int field_render_mode;
	float field_range_min;
	float field_range_max;
	float field_alpha_min;
	float field_alpha_max;
	int corss_section_target = 3;
	bool is_corss_section_effect[3] = { 0 };
	int corss_section_mode_and_or[3] = { 0 };
	CrossSection corss_sections[3];	
};