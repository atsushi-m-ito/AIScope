#pragma once
#include <stdint.h>

template<typename T>
class FieldRGBA {
private:
	uint8_t* m_rgba = nullptr;
	const T* m_correspnd_data = nullptr;
	size_t size_data = 0;
public:
	FieldRGBA() = default;
	~FieldRGBA() {
		delete[] m_rgba;
	};

	const uint8_t* DataRGBA() const {
		return m_rgba;		
	}
	const size_t Size() const {
		return size_data;
	}

	const uint8_t* Convert(const T* field, int grid_x, int grid_y, int grid_z) {
		if ((m_correspnd_data == field) || (m_rgba != nullptr)) {
			return m_rgba;
		}

		if (m_rgba == nullptr) {
			size_data = (size_t)grid_x * (size_t)grid_y * (size_t)grid_z;
			m_rgba = new uint8_t[size_data*4];
		}
		else {
			const size_t next_size = (size_t)grid_x * (size_t)grid_y * (size_t)grid_z;
			if (size_data != next_size) {
				delete[] m_rgba;
				size_data = next_size;
				m_rgba = new uint8_t[size_data * 4];
			}
		}

		TransformToByteColor(field, size_data, m_rgba);

	}

private:

	void TransformToByteColor(const T* field, size_t mesh_sz, uint8_t* out_rgba){
		//色つきテクスチャの生成.

		float* field_s = (float*)out_rgba; //for alignment

		//ソートして上位 some percent を算出//
		//dummyで使う
		float sum = 0.0f;
		for (size_t i = 0; i < mesh_sz; ++i) {
			field_s[i] = (float)field[i];
			sum += field_s[i];
		}
		std::sort(field_s, field_s + mesh_sz, [](const float& a, const float& b) {return a > b; });//降順//
#ifdef _DEBUG
		const float f_max = field_s[0];
		const float f_min = field_s[mesh_sz - 1];
		const float alpha = (log(f_max / 0.01f)) / log(f_max / 0.01);
#endif
		const float total_limit_1 = sum * 0.2;//上位20%
		const float total_limit_2 = sum * 0.5;//上位50%
		float sum2 = 0.0f;
		double range_top = (double)1.0;
		double range_mid = 0.0;
		{
			size_t i = 0;
			for (; i < mesh_sz; ++i) {
				sum2 += field_s[i];
				if (sum2 > total_limit_1) {
					range_top = (double)field_s[i];
					break;
				}
			}
			for (++i; i < mesh_sz; ++i) {
				sum2 += field_s[i];
				if (sum2 > total_limit_2) {
					range_mid = (double)field_s[i];
					break;
				}
			}
		}



		for (size_t k = 0; k < mesh_sz; ++k) {
			//0.6 is the position green color in range of Spectrum//
			double val = 1.0 + 0.6 * ((double)(field[k]) - range_top) / (range_top - range_mid);
			SpectrumColor(val, out_rgba + k * 4, out_rgba + k * 4 + 1, out_rgba + k * 4 + 2);//規格化済み//

			//128 is maximum alpha value(magic number)
			out_rgba[k * 4 + 3] = (val > 1.0) ? 128 : (val < 0.3) ? 0 : (GLubyte)(128.0 * val);

		}
		

	}


#if 0
	/**
	* 値と最大値から色に変換
	* @param value 値
	* @param maxValue 値の最大値
	* @param color 値から求められるBGRA配列
	* by Naoki Kashima
	*/
	void valueToRGBA(double value, double maxValue, uint8_t* rgb) {
		static const int maxHue = 300;
		double f;
		int h;
		int i, p, q, t;
		double opacity;

		if (value < 0)value = 0;
		if (value > maxValue)value = maxValue;
		h = (int)(maxHue * value / maxValue);

		i = (int)floor(h / 60.0) % 6;
		//i = (h * 6)/maxHue;
		f = (double)(h / 60.0) - (double)floor(h / 60.0);
		p = 0;
		q = (int)(255 * (1.0 - f) + 0.5);
		t = (int)(255 * f + 0.5);

		switch (i) {
		case 0:
			opacity = (double)h / 60.0;
			//opacity = 255;
			//rgb[0] = 255; rgb[1] = t; rgb[2] = p; rgb[3] = (unsigned char)opacity;
			rgb[2] = (unsigned char)(255 * opacity); rgb[1] = (unsigned char)(t * opacity); rgb[0]
				= (unsigned char)(p * opacity); rgb[3] = (unsigned char)(255 * opacity);
			break;
		case 1: rgb[2] = q; rgb[1] = 255; rgb[0] = p; rgb[3] = 255; break;
		case 2: rgb[2] = p; rgb[1] = 255; rgb[0] = t; rgb[3] = 255; break;
		case 3: rgb[2] = p; rgb[1] = q; rgb[0] = 255; rgb[3] = 255; break;
		case 4: rgb[2] = t; rgb[1] = p; rgb[0] = 255; rgb[3] = 255; break;
		case 5: rgb[2] = 255; rgb[1] = p; rgb[0] = q; rgb[3] = 255; break;
		}
	}
#endif

	//スペクトル色を返す
	static void SpectrumColor(double value, uint8_t* r, uint8_t* g, uint8_t* b) {
		//	static BYTE tr[11] = {0, 12, 90,160,200,225,240,250,250,250,250};
		//	static BYTE tg[11] = {0,  0,  0,  0,  0, 40, 90,140,190,240,250};
		//	static BYTE tb[11] = {0,120,150,150,120, 10,  0,  0, 10,120,250};
		static uint8_t tr[11] = { 0, 38, 76,  0,  0,  0,  0, 60,120,180,255 };
		static uint8_t tg[11] = { 0,  0,  0,  0,140,180,250,180,120, 60,  0 };
		static uint8_t tb[11] = { 0,106,211,230,120, 60,  0,  0,  0,  0,  0 };

		if (value < 0.0) {
			*r = *g = *b = 0;
		}

		double pos;
		double tt = modf(value * 10.0, &pos);
		int ipos = (int)(pos);
		if (ipos >= 10) {
			*r = tr[10];
			*g = tg[10];
			*b = tb[10];
		}
		else if (ipos < 0) {
			*r = tr[0];
			*g = tg[0];
			*b = tb[0];
		}
		else {
			*r = tr[ipos] + (uint8_t)(tt * (tr[ipos + 1] - tr[ipos]));
			*g = tg[ipos] + (uint8_t)(tt * (tg[ipos + 1] - tg[ipos]));
			*b = tb[ipos] + (uint8_t)(tt * (tb[ipos + 1] - tb[ipos]));
		}

	}

};
