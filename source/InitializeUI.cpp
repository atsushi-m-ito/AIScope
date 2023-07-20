
#include "winmain.h"
#include "UIPanel.h"
#include "MyPanelID.h"

/*

	アプリケーションごとに固有のパネルは位置を書く


*/
void InitializeUI(UIPanel& uipanel)
{

	/*
		ページの基準幅をpixel単位で指定
	*/
	uipanel.SetBaseWidth(64);

	/*
		0番ページ:常に表示されるページ
	*/
	uipanel.AddPage(PAGE_ID_DEFAULT, UIPanel_LEFT_BOTTOM, 1.0, 1.0);
	
	
	PanelRenderInfo panel;
	panel.texture_id = 0;
	panel.source.x1 = 0.0f;
	panel.source.y1 = 0.0f;
	panel.source.x2 = 1.0f;
	panel.source.y2 = 1.0f;

	//基準幅で規格化した表示サイズを指定//
	panel.panel_place = 0;
	panel.draw.x1 = 0.0f;
	panel.draw.y1 = 0.0f;
	panel.draw.x2 = 1.0f;
	panel.draw.y2 = 1.0f;
	uipanel.AddPanel(PAGE_ID_DEFAULT, panel, PANEL_ID_G, PAGE_ID_MOVIE);


	/*
	1番ページ:ムービーページ
	*/
	uipanel.AddPage(PAGE_ID_MOVIE, UIPanel_LEFT_BOTTOM, 1.0, 1.0);


	//基準幅で規格化した表示サイズを指定//
	panel.panel_place = 0;
	panel.draw.x1 = 0.0f;
	panel.draw.y1 = 0.0f;
	panel.draw.x2 = 1.0f;
	panel.draw.y2 = 1.0f;
	uipanel.AddPanel(PAGE_ID_MOVIE, panel, PANEL_ID_G, PAGE_ID_DEFAULT);

	panel.draw.x1 = 1.0f;
	panel.draw.y1 = 0.0f;
	panel.draw.x2 = 2.0f;
	panel.draw.y2 = 1.0f;
	uipanel.AddPanel(PAGE_ID_MOVIE, panel, PANEL_ID_MOVIE_NEXT, PAGE_ID_DEFAULT);


	panel.draw.x1 = 2.0f;
	panel.draw.y1 = 0.0f;
	panel.draw.x2 = 3.0f;
	panel.draw.y2 = 1.0f;
	uipanel.AddPanel(PAGE_ID_MOVIE, panel, PANEL_ID_MOVIE_PREV, PAGE_ID_DEFAULT);


	panel.draw.x1 = 3.0f;
	panel.draw.y1 = 0.0f;
	panel.draw.x2 = 4.0f;
	panel.draw.y2 = 1.0f;
	uipanel.AddPanel(PAGE_ID_MOVIE, panel, PANEL_ID_MOVIE_START, PAGE_ID_DEFAULT);


	panel.draw.x1 = 4.0f;
	panel.draw.y1 = 0.0f;
	panel.draw.x2 = 5.0f;
	panel.draw.y2 = 1.0f;
	uipanel.AddPanel(PAGE_ID_MOVIE, panel, PANEL_ID_MOVIE_BACK, PAGE_ID_DEFAULT);


	panel.draw.x1 = 5.0f;
	panel.draw.y1 = 0.0f;
	panel.draw.x2 = 6.0f;
	panel.draw.y2 = 1.0f;
	uipanel.AddPanel(PAGE_ID_MOVIE, panel, PANEL_ID_MOVIE_STOP, PAGE_ID_DEFAULT);



}