#include "WriteBMP.h"
#include <cmath>

namespace {

	const DWORD SZ_BMFH = 3 * sizeof(WORD) + 2 * sizeof(DWORD);
	const DWORD SZ_BMIH = 2 * sizeof(WORD) + 5 * sizeof(DWORD)
		+ 4 * sizeof(LONG);
}

WriteBMP::WriteBMP(int width, int height, int color_depth) :
	m_buffer(NULL),
	m_width(width),
	m_height(height),
	m_color_depth(color_depth),
	m_stride(((width * color_depth + 3) / 4) * 4)
{

	m_bmih.biSize = 40;
	m_bmih.biWidth = width;
	m_bmih.biHeight = height;
	m_bmih.biPlanes = 1;
	m_bmih.biBitCount = color_depth * 8;
	m_bmih.biCompression = 0;
	m_bmih.biSizeImage = m_stride * height;
	m_bmih.biXPelsPerMeter = 0;
	m_bmih.biYPelsPerMeter = 0;
	m_bmih.biClrUsed = 0;
	m_bmih.biClrImportant = 0;

}

WriteBMP::~WriteBMP()
{
	if (m_buffer) delete[] m_buffer;

}


void WriteBMP::CreateDIBfromRGBA(BYTE* rgbaBuf, int src_pitch, int offset_x, int offset_y, int w, int h)
{

	if (m_buffer == NULL) {
		m_buffer = new BYTE[m_bmih.biSizeImage];
	}

	const int offset = offset_x * m_color_depth + (m_height - h - offset_y) * m_stride;
	int rk = h * src_pitch;
	for (int dk = 0; dk < h * m_stride; dk += m_stride) {
		rk -= src_pitch;
		for (int i = 0; i < w; i++) {
			int di = i * 3 + dk + offset;
			int ri = i * 4 + rk;

			m_buffer[di] = rgbaBuf[ri + 2];
			m_buffer[di + 1] = rgbaBuf[ri + 1];
			m_buffer[di + 2] = rgbaBuf[ri];

		}
	}


}

float WriteBMP::FP32FromFP16(const void* fp16) {

	unsigned short *ui16 = (unsigned short *)fp16;
	union {
		unsigned int ui32;
		float fp32;
	};

	ui32 = ((unsigned int)((*ui16) & 0x3FF) << 13) | (((unsigned int)(((*ui16) & 0x7C00) >> 10) - 15 + 127) << 23) | ((unsigned int)((*ui16) & 0x8000) << 16);

	return fp32;


}

/*
sRGBへ変換
*/
BYTE WriteBMP::sRGBFromFP16(const void* fp16) {
	const float i_gamma = (1.0f / 2.2f);
	const float power = std::pow(FP32FromFP16(fp16), i_gamma);
	if (power > 1.0f) {
		return 255;
	} else {
		return (BYTE)(255.0f * power);
	}
}

void WriteBMP::CreateDIBfromRGBA_FP16(BYTE* rgbaBuf, int src_pitch, int offset_x, int offset_y, int w, int h) 
{

	if (m_buffer == NULL) {
		m_buffer = new BYTE[m_bmih.biSizeImage];
	}

	const int offset = offset_x * m_color_depth + (m_height - h - offset_y) * m_stride;
	int rk = h * src_pitch;
	for (int dk = 0; dk < h * m_stride; dk += m_stride) {
		rk -= src_pitch;
		for (int i = 0; i < w; i++) {
			int di = i * 3 + dk + offset;
			int ri = i * 8 + rk;

			m_buffer[di] = sRGBFromFP16(rgbaBuf + ri + 4);
			m_buffer[di + 1] = sRGBFromFP16(rgbaBuf + ri + 2);
			m_buffer[di + 2] = sRGBFromFP16(rgbaBuf + ri);

			/*
			dibBuf[di] = ByteFromFP16(rgbaBuf + ri + 4);
			dibBuf[di + 1] = ByteFromFP16(rgbaBuf + ri +2);
			dibBuf[di + 2] = ByteFromFP16(rgbaBuf + ri);
			*/
			/*
			dibBuf[di] = (rgbaBuf[ri + 5] & 0x7F) << 1;
			dibBuf[di + 1] = (rgbaBuf[ri + 3] & 0x7F) << 1;
			dibBuf[di + 2] = (rgbaBuf[ri + 1] & 0x7F) << 1;
			*/
		}
	}


}


void WriteBMP::WriteDIB(FILE *fp) {
	/*
	if (pbmih->biSizeImage) {
		size_Line = pbmih->biSizeImage / pbmih->biHeight;
	} else {
		size_Line = pbmih->biWidth * (pbmih->biBitCount / 8);
		if (size_Line % 4) { size_Line += 4 - (size_Line % 4); }
		pbmih->biSizeImage = size_Line * pbmih->biHeight;
	}
	*/

	BITMAPFILEHEADER bmfh;
	bmfh.bfType = 0x4D42;
	bmfh.bfSize = SZ_BMFH + m_bmih.biSize + m_bmih.biSizeImage;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = SZ_BMFH + m_bmih.biSize;

	writeBMH(&bmfh, &m_bmih, fp);

	fwrite(m_buffer, m_bmih.biSizeImage, 1, fp);

	return;
}

