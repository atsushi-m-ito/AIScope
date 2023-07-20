#pragma once

#ifndef UIPanel_H
#define UIPanel_H

#include <vector>
#include <map>
#include "INotifyProperty.h"


struct rect4f{
	float x1;
	float y1;
	float x2;
	float y2;
};

struct PanelRenderInfo{
	int texture_id;
	int panel_place;
	rect4f source;
	rect4f draw;
};

enum UIPanelPagePlace{
	UIPanel_CENTER, UIPanel_BOTTOM, UIPanel_TOP, UIPanel_LEFT_BOTTOM
};


class UIPanel : public INotifyProperty
{
private:

	int m_current_page_id;
	int m_base_width;

	struct PanelAction{
		int panel_id;
		int next_page;		//-1ならページ変更しない, 0ならページを閉じる.

	};

	struct Page{
		UIPanelPagePlace place;
		float width;
		float height;
		std::vector<PanelRenderInfo> button;
		std::vector<PanelAction> actions;
	};

	std::map<int, Page> pages;
	

public:

	UIPanel();
	virtual ~UIPanel();
	
//UIウィジェットに関する登録関連///////////////////////////////
	void SetBaseWidth(int base_width){
		m_base_width = base_width;
	}

	int AddPage(int page_id, UIPanelPagePlace place, float w, float h){
		if (page_id >= 0){
			Page& page = pages[page_id];
			page.place = place;
			page.width = w;
			page.height = h;

			return page_id;
		}
		return -1;
	}

	void AddPanel(int page_id, const PanelRenderInfo& button, int panel_id, int next_page){
		if (panel_id > 0) {
			pages[page_id].button.push_back(button);

			PanelAction action;
			action.panel_id = panel_id;
			action.next_page = next_page;
			pages[page_id].actions.push_back(action);
		}
	};


//ウィジェットのクリック判定関連(システムから呼ぶ)//////////////////////////////////////
	/*
	ページを押したかどうかのチェック
	0以上の値が帰れば押されている
	*/
	int HitPage(int x, int y, int w, int h) {
		auto itr = pages.find(m_current_page_id);
		if (itr == pages.end()){
			return -1;
		}

		const Page& page = itr->second;

		float fx, fy;


		/*
		パネル内の相対座標fx,fyに変換
		パネル外部をクリックしていた場合は何もしない
		*/
		switch (page.place){
		case UIPanel_CENTER:
			fx = (float)(x - w / 2) / (float)m_base_width + page.width * 0.5f;
			fy = (float)(y - h / 2) / (float)m_base_width + page.height * 0.5f;

			if ((0.0f <= fx) && (fx < page.width) && (0.0f <= fy) && (fy < page.height)){
				return 1;
			}
			break;
		case UIPanel_LEFT_BOTTOM:
			fx = (float)(x) / (float)m_base_width;
			fy = (float)(y - h) / (float)m_base_width + page.height;

			if ((0.0f <= fx) && (fx < page.width) && (0.0f <= fy) && (fy < page.height)){
				return 1;
			}
			break;

		case UIPanel_BOTTOM:
			fx = (float)(x) / (float)m_base_width;
			fy = (float)(y - h) / (float)m_base_width + page.height;
			if ((0.0f <= fy) && (fy < page.height)){
				return 1;
			}
			break;

		case UIPanel_TOP:
			fx = (float)(x) / (float)m_base_width;
			fy = (float)(y) / (float)m_base_width;
			if ((0.0f <= fy) && (fy < page.height)){
				return 1;
			}
			break;
		}


		return -1;
	};

	/*
		パネルを押したかどうかのチェック
		0以上の値が帰れば押されている
	*/
	int HitPanel(int x, int y, int w, int h) {
		auto itr = pages.find(m_current_page_id);
		if (itr == pages.end()){
			return -1;
		}

		const Page& page = itr->second;
		
		float fx, fy;
		
		
		/*
			パネル内の相対座標fx,fyに変換
		*/
		switch (page.place){
		case UIPanel_CENTER:
			fx = (float)(x - w / 2) / (float)m_base_width + page.width * 0.5f;
			fy = (float)(y - h / 2) / (float)m_base_width + page.height * 0.5f;
			
			break;
		case UIPanel_LEFT_BOTTOM:
			fx = (float)(x) / (float)m_base_width;
			fy = (float)(y - h) / (float)m_base_width + page.height;

			break;

		case UIPanel_BOTTOM:
			fx = (float)(x) / (float)m_base_width;
			fy = (float)(y - h) / (float)m_base_width + page.height;
			break;
		
		case UIPanel_TOP:
			fx = (float)(x) / (float)m_base_width;
			fy = (float)(y) / (float)m_base_width;
			break;
		}



		/*
			ページ内で押されているパネルを探す.
		*/
		const int i_end = page.actions.size();
		for (int i = 0; i < i_end; ++i){
			if (mCheckOnPanel(page.button[i].panel_place, page.button[i].draw, fx, fy)){

				switch (page.actions[i].next_page){
				case 0:
					m_current_page_id = 0;
					
					break;
				case -1:
					//何もしない//
					break;
				default:
					m_current_page_id = page.actions[i].next_page;
					break;
				}
				return page.actions[i].panel_id;
			}
		}
		
		return -1;
	};

	int mCheckOnPanel(int panel_place, const rect4f& panel, float fx, float fy){
		if ((panel.x1 <= fx) && (fx < panel.x2) && (panel.y1 <= fy) && (fy < panel.y2)){
			return 1;
		}
		return 0;
	};

//UIウィジェットの描画に関する情報取得(Rendererから呼ぶ)///////////////////////////////

	int GetDrawInfomation(const int screen_w, const int screen_h, double* base_width, rect4f* page_rect, const std::vector<PanelRenderInfo>** panels) const
	{
		auto itr = pages.find(m_current_page_id);
		if (itr == pages.end()){
			return -1;
		}

		const Page& page = itr->second;

		*base_width = (float)m_base_width;
		const float ratio_x = (float)screen_w / (float)m_base_width;
		const float ratio_y = (float)screen_h / (float)m_base_width;
		
		switch (page.place){
		case UIPanel_CENTER:
			page_rect->x1 = -page.width * 0.5f + ratio_x * 0.5f;
			page_rect->y1 = -page.height * 0.5f + ratio_y * 0.5f;
			page_rect->x2 = page.width * 0.5f+ ratio_x * 0.5f;
			page_rect->y2 = page.height * 0.5f + ratio_y * 0.5f;

			break;
		case UIPanel_LEFT_BOTTOM:
			page_rect->x1 = 0.0f;
			page_rect->y1 = -page.height + ratio_y;
			page_rect->x2 = page.width;
			page_rect->y2 = ratio_y;


			break;
		case UIPanel_BOTTOM:
			page_rect->x1 = 0.0f;
			page_rect->y1 = -page.height + ratio_y;
			page_rect->x2 = ratio_x;
			page_rect->y2 = ratio_y;

			
			break;
		case UIPanel_TOP:
			page_rect->x1 = 0.0f;
			page_rect->y1 = 0.0;
			page_rect->x2 = ratio_x;
			page_rect->y2= page.height;

			break;
		}

		*panels = &(page.button);

		return 1;
	};

//ウィジェットと結びついたプロパティーの変更通知を受ける窓口(プロパティー変更時に呼ばれる)////////////////////////////
	void Notify(int key, int value){

	};

	/*
	//処理しないものは何もしないようにする//
	template<typename T>
	void Notify(int key, T value){
	};

	//実装は特殊化で///////////////////
	template<>
	void Notify(int key, int value){

	};

	//実装は特殊化で///////////////////
	template<>
	void Notify(int key, double value){

	};
	*/

};

#endif