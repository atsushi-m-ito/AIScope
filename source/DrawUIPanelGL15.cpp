#include "RenderGL15.h"
#include "UIPanel.h"

void DrawUIPanelGL15(int screen_w, int screen_h, const UIPanel& uipanel){

	double base_width;
	rect4f page_rect;
	const std::vector<PanelRenderInfo>* panels;
	int res = uipanel.GetDrawInfomation(screen_w, screen_h, &base_width, &page_rect, &panels);
	if (res == -1){
		return;
	}

	//SetLightGL();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, ((double)screen_w) / base_width, ((double)screen_h) / base_width, 0.0, 1.0, -1.0);
	glViewport(0, 0, screen_w, screen_h);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);


	glColor4d(0.0, 0.0, 0.0, 0.5);


	glBegin(GL_QUADS);
	glVertex2d(page_rect.x1, page_rect.y1);
	glVertex2d(page_rect.x1, page_rect.y2);
	glVertex2d(page_rect.x2, page_rect.y2);
	glVertex2d(page_rect.x2, page_rect.y1);
	glEnd();

	
	glDisable(GL_BLEND);

	glColor4d(1.0, 0.0, 0.0, 1.0);


	//glEnable(GL_TEXTURE_2D);
	for (auto itr = panels->begin(); itr != panels->end(); ++itr){
		
		//glBindTexture(GL_TEXTURE_2D, itr->texture_id);

		const rect4f& src = itr->source;
		const rect4f& draw = itr->draw;
		rect4f dest;
		dest.x1 = draw.x1 + page_rect.x1;
		dest.x2 = draw.x2 + page_rect.x1;
		dest.y1 = draw.y1 + page_rect.y1;
		dest.y2 = draw.y2 + page_rect.y1;


		glBegin(GL_QUADS);
//		glTexCoord2f(src.x1, src.y1);
		glVertex2d(dest.x1, dest.y1);
	//	glTexCoord2f(src.x1, src.y2);
		glVertex2d(dest.x1, dest.y2);
		//glTexCoord2f(src.x2, src.y2);
		glVertex2d(dest.x2, dest.y2);
		//glTexCoord2f(src.x2, src.y1);
		glVertex2d(dest.x2, dest.y1);
		glEnd();

	}
	

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}