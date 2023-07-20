#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <list>
#include <tuple>
#include <vector>

#include "vec3.h"
#include "mat33.h"


class STL_Grid_Writer
{

private:
	FILE * m_fp{ nullptr };

public:
	virtual ~STL_Grid_Writer() {
		Close();
	}

	bool Open(const char* file_path) {
		m_fp = fopen(file_path, "wb");
		//m_fp = fopen(file_path, "w");
		return ((!m_fp) ? false : true);
	}

	void Close() {
		if (m_fp) fclose(m_fp);
	}

	//template<typename T>
	int Write(const vec3f* r, int count, double radius, mat33d box_axis, double min_grid_width, int cutoff_edge_z, double scale);

private:
	int mFolding(int x, int num_grid) {
#if 1
		if(x < 0) {
			x += num_grid;
		}else if (x >= num_grid) {
			x -= num_grid;
		}
#else
		while (x < 0) {
			x += num_grid;
		}
		while (x >= num_grid) {
			x -= num_grid;
		}
#endif
		return x;
	}

	struct Line
	{
		std::tuple<int8_t, int8_t> begin;
		std::tuple<int8_t, int8_t> end;
	};
	
	//4点で構成された四角形面において、断面と交わる線分をlinesに追加//
	void mSetLineInSurface(const int8_t vertexes[4], const bool is_full_list[8], std::list<Line>& lines) {
				
		const bool v_is_full[4]{ is_full_list[vertexes[0]],is_full_list[vertexes[1]],is_full_list[vertexes[2]],is_full_list[vertexes[3]] };
		
		for (int offset = 0; offset < 4; ++offset) {
			if (!v_is_full[offset]) {
				bool start_is_found = false;
				std::tuple<int8_t, int8_t> begin_line;
				int next = offset;
				for (int i = 0; i < 4; ++i) {
					const int k = next;
					next = (next + 1) % 4;
					if (!start_is_found) {
						if (v_is_full[next]) {
							start_is_found = true;
							begin_line = { std::min<int8_t>(vertexes[k] , vertexes[next]), std::max<int8_t>(vertexes[k] , vertexes[next]) };
						}
					}else{
						if (!v_is_full[next]) {
							start_is_found = false;
							lines.push_back({ begin_line,{ std::min<int8_t>(vertexes[k] , vertexes[next]), std::max<int8_t>(vertexes[k] , vertexes[next]) } });
						}
					}
				}
				break;
			}
		}
		
	}

	vec3d mPositionOnLine(std::tuple<int8_t, int8_t> pos, const int grid_ids[8], const vec3d* grid_cut) {
		const int id = std::get<0>(pos);// std::min<int>(std::get<0>(pos), std::get<1>(pos));
		const int i = grid_ids[id];
		switch (std::get<0>(pos)){
		case 0:
			switch (std::get<1>(pos)) {
			case 1:
				return { grid_cut[i].x, 0.0, 0.0 };
			case 2:
				return { 0.0, grid_cut[i].y, 0.0 };
			case 4:
				return { 0.0, 0.0, grid_cut[i].z };
			}
		case 1:
			switch (std::get<1>(pos)) {
			case 3:
				return { 1.0, grid_cut[i].y, 0.0 };
			case 5:
				return { 1.0, 0.0, grid_cut[i].z };
			}
		case 2:
			switch (std::get<1>(pos)) {
			case 3:
				return { grid_cut[i].x, 1.0, 0.0 };
			case 6:
				return { 0.0, 1.0, grid_cut[i].z };
			}
		case 3:
			switch (std::get<1>(pos)) {
			case 7:				
				return { 1.0, 1.0, grid_cut[i].z };
			}
		case 4:
			switch (std::get<1>(pos)) {
			case 5:
				return { grid_cut[i].x, 0.0, 1.0 };
			case 6:
				return { 0.0, grid_cut[i].y, 1.0 };
			}
		case 5:
			switch (std::get<1>(pos)) {
			case 7:
				return { 1.0, grid_cut[i].y, 1.0};
			}
		case 6:
			switch (std::get<1>(pos)) {
			case 7:
				return { grid_cut[i].x, 1.0, 1.0 };
			}

		}
	}


	vec3d mPositionOnVertex(int8_t pos) {
		return { (pos & 0x1) ? 1.0 : 0.0,
			(pos & 0x2) ? 1.0 : 0.0,
			(pos & 0x4) ? 1.0 : 0.0 };
			
	}

	int mPolygonInUnitcell(const int grid_ids[8], const bool v_is_full[8], const vec3d* grid_cut, const vec3d& cell_org,  std::vector<vec3d>& poly_list) {
		
		std::list<Line> lines;


		//6つの面に断面が交差した際の線分をlinesに記録//
		{
			const int8_t vertexes[4] = { 0,2,3,1 };//surface 1//
			mSetLineInSurface(vertexes, v_is_full, lines);
		}
		{
			const int8_t vertexes[4] = { 0,1,5,4 };
			mSetLineInSurface(vertexes, v_is_full, lines);
		}
		{
			const int8_t vertexes[4] = { 0,4,6,2 };
			mSetLineInSurface(vertexes, v_is_full, lines);
		}
		{
			const int8_t vertexes[4] = { 7,6,4,5 };
			mSetLineInSurface(vertexes, v_is_full, lines);
		}
		{
			const int8_t vertexes[4] = { 7,5,1,3 };
			mSetLineInSurface(vertexes, v_is_full, lines);
		}
		{
			const int8_t vertexes[4] = { 7,3,2,6 };
			mSetLineInSurface(vertexes, v_is_full, lines);
		}

		//linesの繋がりから断面を作成//
		int num_polygon = 0;
		while (!lines.empty()) {
			std::list<Line> poly;
			poly.splice(poly.begin(), lines, lines.begin());

			auto last_end = poly.back().end;
			while (last_end != poly.front().begin) {
				for (auto it = lines.begin(); it != lines.end(); it++) {
					if (last_end == it->begin) {
						last_end = it->end;
						poly.splice(poly.end(), lines, it);
						break;
					}
				}
			}
			//ここに来たら閉じたlinesになっているはず//
			//ポリゴンを追加//
			auto first_pos = poly.front().begin;
			const auto it_end = std::prev(poly.end());
			for (auto it = std::next(poly.begin()); it != it_end; ++it) {
				const auto& second_pos = it->begin;
				const auto& third_pos = it->end;
				poly_list.push_back(cell_org + mPositionOnLine(first_pos, grid_ids, grid_cut));
				poly_list.push_back(cell_org + mPositionOnLine(second_pos, grid_ids, grid_cut));
				poly_list.push_back(cell_org + mPositionOnLine(third_pos, grid_ids, grid_cut));
				++num_polygon;
			}
		}

		return num_polygon;
	}

	void mPolygonBoundary(const int8_t vertexes[4], const bool v_is_full[4], const int grid_ids[4], const vec3d* grid_cut, const vec3d& cell_org, std::vector<vec3d>& poly_list) {


		if (v_is_full[0] && v_is_full[1] && v_is_full[2] && v_is_full[3]) {
			poly_list.push_back(cell_org + mPositionOnVertex(vertexes[0]));
			poly_list.push_back(cell_org + mPositionOnVertex(vertexes[1]));
			poly_list.push_back(cell_org + mPositionOnVertex(vertexes[2]));
			
			poly_list.push_back(cell_org + mPositionOnVertex(vertexes[0]));
			poly_list.push_back(cell_org + mPositionOnVertex(vertexes[2]));
			poly_list.push_back(cell_org + mPositionOnVertex(vertexes[3]));


		} else {

			for (int offset = 0; offset < 4; ++offset) {
				if (!v_is_full[offset]) {
					bool start_is_found = false;
					std::tuple<int8_t, int8_t> begin_line;
					int next = offset;
					for (int i = 0; i < 4; ++i) {
						const int k = next;
						next = (next + 1) % 4;
						if (!start_is_found) {
							if (v_is_full[next]) {
								start_is_found = true;
								begin_line = { std::min<int8_t>(vertexes[k] , vertexes[next]), std::max<int8_t>(vertexes[k] , vertexes[next]) };
								poly_list.push_back(cell_org + mPositionOnLine(begin_line, grid_ids, grid_cut));
								poly_list.push_back(cell_org + mPositionOnVertex(vertexes[next]));
							}
						} else {
							if (!v_is_full[next]) {
								start_is_found = false;
								//end of triangle
								poly_list.push_back(cell_org + mPositionOnLine({ std::min<int8_t>(vertexes[k] , vertexes[next]), std::max<int8_t>(vertexes[k] , vertexes[next]) }, grid_ids, grid_cut));
							} else {
								//end of triangle
								poly_list.push_back(cell_org + mPositionOnVertex(vertexes[next]));

								//begin next triangle
								poly_list.push_back(cell_org + mPositionOnLine(begin_line, grid_ids, grid_cut));
								poly_list.push_back(cell_org + mPositionOnVertex(vertexes[next]));

							}
						}
					}
					break;
				}
			}
		}
	}


};

