#pragma once

/*****************************************************************

アプリケーション固有のプロパティーを管理する.

ChangePropertyでプロパティー値を受け取り,

INotifyProperty* m_reciever;
として登録されているレシーバーに通知(Notify)する.

RegisterNotifyReciever()によってm_recieverにレシーバーを登録する.

******************************************************************/


#include <cstdlib>
#include "INotifyProperty.h"
#include "PropertyList.h"

#include "RenderingProperty.h"


class AIScopeProperty
{
private:
	INotifyProperty* m_reciever;

	//property一覧//
	RenderingProperty m_rendering_property;
	
public:
	AIScopeProperty() :
		m_reciever(nullptr)
	{
	};


	void RegisterNotifyReciever(INotifyProperty* reciever){
		m_reciever = reciever;
	}

	void ChangeProperty(int key, int value)
	{
		if (mSetPropertyInList(key, value) > 0){
			//成功した場合//
			if (m_reciever)	{
				m_reciever->Notify(key, value);
			}
		}

	}

	void ChangePropertyDouble(int key, double value)
	{
		if (mSetPropertyInListDouble(key, value) > 0) {
			//成功した場合//
			if (m_reciever) {
				m_reciever->Notify(key, value);
			}
		}

	}

private:
	int mSetPropertyInList(int key, int value){

		/*
			アプリケーション固有のプロパティーを記述
		*/
		switch (key){
		case AISCUPE_KEY_PROJECTION:
			m_rendering_property.projection_mode = value;
			break;
		case AISCUPE_KEY_BOX_DRAW:
			m_rendering_property.box_draw = value;
			break;
		case AISCUPE_KEY_MULTIVIEW:
			m_rendering_property.multiview_mode = value;
			break;
		case AISCUPE_KEY_ATOM_DRAW:
			m_rendering_property.atom_draw = value;
			break;
		case AISCUPE_KEY_ATOM_COLOR:
			m_rendering_property.atom_color = value;
			break;
		case AISCUPE_KEY_BOND_DRAW:
			m_rendering_property.bond_draw = value;
			break;
		case AISCUPE_KEY_BOND_POLY:
			m_rendering_property.bond_poly = value;
			break;
		case AISCUPE_KEY_ATOM_POLY:
			m_rendering_property.atom_poly = value;
			break;
		case AISCUPE_KEY_FIELD_RENDERMODE:
			m_rendering_property.field_render_mode = value;
		default:
			return -1;
		}

		return key;
	}

	int mSetPropertyInListDouble(int key, double value) {

		/*
			アプリケーション固有のプロパティーを記述
		*/
		switch (key) {
		case AISCUPE_KEY_ATOM_RADIUS:
			m_rendering_property.atom_radius = value;
			break;
		case AISCUPE_KEY_BG_COLOR_R:
			m_rendering_property.bg_color[0] = value;
			break;
		case AISCUPE_KEY_BG_COLOR_G:
			m_rendering_property.bg_color[1] = value;
			break;
		case AISCUPE_KEY_BG_COLOR_B:
			m_rendering_property.bg_color[2] = value;
			break;
		case AISCUPE_KEY_BG_COLOR_A:
			m_rendering_property.bg_color[3] = value;
			break;
		case AISCUPE_KEY_FIELD_RANGE_MIN:
			m_rendering_property.field_range_min = value;
			break;
		case AISCUPE_KEY_FIELD_RANGE_MAX:
			m_rendering_property.field_range_max = value;
			break;
		case AISCUPE_KEY_FIELD_ALPHA_MIN:
			m_rendering_property.field_alpha_min = value;
			break;
		case AISCUPE_KEY_FIELD_ALPHA_MAX:
			m_rendering_property.field_alpha_max = value;
			break;

		default:
			return -1;
		}

		return key;
	}
public:
	//3Dレンダリング時に使用するプロパティー値を返す//
	void GetRenderingProperty(RenderingProperty* rendering_property){
		if (rendering_property == NULL){
			return;
		}

		*rendering_property = m_rendering_property;

	};


	bool SetCrossSection(int number, bool effective, const vec3d point, const vec3d& normal, int and_or) {
		if (number < 0)return false;
		if (number >= 3)return false;
		if (effective) {
			m_rendering_property.corss_sections[number].point.Set(point.x,point.y,point.z,0.0f);
			m_rendering_property.corss_sections[number].normal.Set(normal.x, normal.y, normal.z,0.0f);
		}
		m_rendering_property.corss_section_mode_and_or[number] = and_or;
		if (m_rendering_property.is_corss_section_effect[number] == effective) return false;
		m_rendering_property.is_corss_section_effect[number] = effective;
		
		return true;
	}


	bool SetCrossSectionTarget(int target) {
		if ((target == 1) || (target == 3)) {
			m_rendering_property.corss_section_target = target;
			return true;
		}
		return false;
	}
};