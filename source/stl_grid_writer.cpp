
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
#include "stl_grid_writer.h"

#if 0
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
		return ((!m_fp) ? false : true);
	}

	void Close() {
		if (m_fp) fclose(m_fp);
	}
#endif
//template<typename T>
int STL_Grid_Writer::Write(const vec3f* r, int count, double radius, mat33d box_axis, double min_grid_width, int cutoff_edge_z, double scale) {

		//解像度決定
		const double max_box_width = std::max<double>(std::max<double>(box_axis.a.x, box_axis.b.y), box_axis.c.z);
		const int num_grids_x = (int)(box_axis.a.x / min_grid_width);
		const int num_grids_y = (int)(box_axis.b.y / min_grid_width);
		const int num_grids_z = (int)(box_axis.c.z / min_grid_width);
		const double grid_width_x = box_axis.a.x / num_grids_x;
		const double grid_width_y = box_axis.b.y / num_grids_y;
		const double grid_width_z = box_axis.c.z / num_grids_z;
		const int num_total_grids = num_grids_x * num_grids_y * num_grids_z;

		bool* grids_is_full = new bool[num_total_grids];
		vec3d* grids_cut = new vec3d[num_total_grids];
		vec3d* grids_back = new vec3d[num_total_grids];
		
		for (int i = 0; i < num_total_grids; ++i) {
			grids_is_full[i] = false;
			grids_cut[i].Clear();
			grids_back[i].Clear();
		}
		

		//粒子毎に半径内部をfillする//
		for (int i = 0; i < count; ++i) {
			vec3d rt(r[i].x, r[i].y, r[i].z);
			if (rt.x < 0.0 || rt.x >= box_axis.a.x) rt.x -= floor(rt.x / box_axis.a.x) * box_axis.a.x;
			if (rt.y < 0.0 || rt.y >= box_axis.b.y) rt.y -= floor(rt.y / box_axis.b.y) * box_axis.b.y;
			if (rt.z < 0.0 || rt.z >= box_axis.c.z) rt.z -= floor(rt.z / box_axis.c.z) * box_axis.c.z;

			const int begin_x = (int)floor((rt.x - radius) / grid_width_x);
			const int end_x = (int)ceil((rt.x + radius) / grid_width_x);
			const int begin_y = (int)floor((rt.y - radius) / grid_width_y);
			const int end_y = (int)ceil((rt.y + radius) / grid_width_y);
			const int begin_z = (int)floor((rt.z - radius) / grid_width_z);
			const int end_z = (int)ceil((rt.z + radius) / grid_width_z);

			const double radius_sq = radius * radius;
			for (int iz = begin_z; iz < end_z; ++iz) {
				const int kz = mFolding(iz, num_grids_z);
				for (int iy = begin_y; iy < end_y; ++iy) {
					const int ky = mFolding(iy, num_grids_y);
					//各グリッド線との交点を算出//

					const double dy = grid_width_y * (double)iy - rt.y;
					const double dz = grid_width_z * (double)iz - rt.z;

					const double dx2 = radius_sq - dy * dy - dz * dz;
					if (dx2 > 0.0) {
					//x軸に平行なグリッド線との交点//
						const double cx = sqrt(dx2);
						const double cx_minus = (rt.x - cx) / grid_width_x; //munus side cross point//
						const double cx_plus = (rt.x + cx) / grid_width_x; //plus side cross point//
						int ix_minus = (int)ceil(cx_minus);
						int ix_plus = (int)floor(cx_plus); //munus side cross point//
						if (ix_minus == ix_plus + 1) {
							grids_is_full[ix_plus + num_grids_x * (ky + num_grids_y * kz)] = true;
						} else {
							const double x_minus_position = cx_minus - (double)ix_minus;
							const double x_plus_position = cx_plus - (double)ix_plus;
							ix_minus = mFolding(ix_minus, num_grids_x); //munus side cross point//
							ix_plus = mFolding(ix_plus, num_grids_x); //munus side cross point//


							if (ix_minus <= ix_plus) {
								for (int k = ix_minus; k <= ix_plus; ++k) {
									grids_is_full[k + num_grids_x * (ky + num_grids_y * kz)] = true;
								}
							} else {
								for (int k = 0; k <= ix_plus; ++k) {
									grids_is_full[k + num_grids_x * (ky + num_grids_y * kz)] = true;
								}
								for (int k = ix_minus; k < num_grids_x; ++k) {
									grids_is_full[k + num_grids_x * (ky + num_grids_y * kz)] = true;
								}
							}

							if (grids_back[ix_minus + num_grids_x * (ky + num_grids_y * kz)].x > x_minus_position) {
								grids_back[ix_minus + num_grids_x * (ky + num_grids_y * kz)].x = x_minus_position;
							}

							if (grids_cut[ix_plus + num_grids_x * (ky + num_grids_y * kz)].x < x_plus_position) {
								grids_cut[ix_plus + num_grids_x * (ky + num_grids_y * kz)].x = x_plus_position;
							}
						}
					}
					
				}
			}


			for (int iz = begin_z; iz < end_z; ++iz) {
				const int kz = mFolding(iz, num_grids_z);
				for (int ix = begin_x; ix < end_x; ++ix) {
					const int kx = mFolding(ix, num_grids_x);
					//各グリッド線との交点を算出//

					const double dx = grid_width_x * (double)ix - rt.x;
					const double dz = grid_width_z * (double)iz - rt.z;


					const double dy2 = radius_sq - dx * dx - dz * dz;
					if (dy2 > 0.0) {
						//y軸に平行なグリッド線との交点//
						const double cy = sqrt(dy2);
						const double cy_minus = (rt.y - cy)/ grid_width_y; //munus side cross point//
						const double cy_plus = (rt.y + cy)/ grid_width_y; //plus side cross point//
						int iy_minus = (int)ceil(cy_minus); //munus side cross point//
						int iy_plus = (int)floor(cy_plus); //munus side cross point//
						if (iy_minus == iy_plus + 1) {
							grids_is_full[kx + num_grids_x * (iy_plus + num_grids_y * kz)] = true;
						} else {
							const double y_minus_position = cy_minus - (double)iy_minus;
							const double y_plus_position = cy_plus - (double)iy_plus;
							iy_minus = mFolding(iy_minus, num_grids_y); //munus side cross point//
							iy_plus = mFolding(iy_plus, num_grids_y); //munus side cross point//


							if (iy_minus <= iy_plus) {
								for (int k = iy_minus; k <= iy_plus; ++k) {
									grids_is_full[kx + num_grids_x * (k + num_grids_y * kz)] = true;
								}
							} else {

								for (int k = 0; k <= iy_plus; ++k) {
									grids_is_full[kx + num_grids_x * (k + num_grids_y * kz)] = true;
								}
								for (int k = iy_minus; k < num_grids_y; ++k) {
									grids_is_full[kx + num_grids_x * (k + num_grids_y * kz)] = true;
								}

							}


							if (grids_back[kx + num_grids_x * (iy_minus + num_grids_y * kz)].y > y_minus_position) {
								grids_back[kx + num_grids_x * (iy_minus + num_grids_y * kz)].y = y_minus_position;
							}

							if (grids_cut[kx + num_grids_x * (iy_plus + num_grids_y * kz)].y < y_plus_position) {
								grids_cut[kx + num_grids_x * (iy_plus + num_grids_y * kz)].y = y_plus_position;
							}
						}
					}
				}
			}


			for (int iy = begin_y; iy < end_y; ++iy) {
				const int ky = mFolding(iy, num_grids_y);
				for (int ix = begin_x; ix < end_x; ++ix) {
					const int kx = mFolding(ix, num_grids_x);

					const double dx = grid_width_x * (double)ix - rt.x;
					const double dy = grid_width_y * (double)iy - rt.y;

					const double dz2 = radius_sq - dx * dx - dy * dy;
					if (dz2 > 0.0) {
						//z軸に平行なグリッド線との交点//
						const double cz = sqrt(dz2);
						const double cz_minus = (rt.z - cz) / grid_width_z; //munus side cross point//
						const double cz_plus = (rt.z + cz) / grid_width_z; //plus side cross point//
						int iz_minus = (int)ceil(cz_minus);
						int iz_plus = (int)floor(cz_plus);
						if (iz_minus == iz_plus + 1) {
							grids_is_full[kx + num_grids_x * (ky + num_grids_y * iz_plus)] = true;
						} else {
							const double z_minus_position = cz_minus - (double)iz_minus;
							const double z_plus_position = cz_plus - (double)iz_plus;
							iz_minus = mFolding(iz_minus, num_grids_z); //munus side cross point//
							iz_plus = mFolding(iz_plus, num_grids_z); //munus side cross point//

							if (iz_minus <= iz_plus) {
								for (int k = iz_minus; k <= iz_plus; ++k) {
									grids_is_full[kx + num_grids_x * (ky + num_grids_y * k)] = true;
								}
							} else {
								for (int k = 0; k <= iz_plus; ++k) {
									grids_is_full[kx + num_grids_x * (ky + num_grids_y * k)] = true;
								}
								for (int k = iz_minus; k < num_grids_z; ++k) {
									grids_is_full[kx + num_grids_x * (ky + num_grids_y * k)] = true;
								}
							}

							if (grids_back[kx + num_grids_x * (ky + num_grids_y * iz_minus)].z > z_minus_position) {
								grids_back[kx + num_grids_x * (ky + num_grids_y * iz_minus)].z = z_minus_position;
							}

							if (grids_cut[kx + num_grids_x * (ky + num_grids_y * iz_plus)].z < z_plus_position) {
								grids_cut[kx + num_grids_x * (ky + num_grids_y * iz_plus)].z = z_plus_position;
							}
						}
					}
				}
			}
		}


		for (int iz = 0; iz < num_grids_z; ++iz) {
			for (int iy = 0; iy < num_grids_y; ++iy) {
				for (int ix = 0; ix < num_grids_x; ++ix) {
					const int index = ix + num_grids_x * (iy + num_grids_y * iz);
					if (!grids_is_full[index]) {
						const int i_next_x = index + (ix + 1 == num_grids_x ? 1 - num_grids_x : 1);
						const int i_next_y = index + num_grids_x * (iy + 1 == num_grids_y ? 1 - num_grids_y : 1);
						const int i_next_z = index + num_grids_x * num_grids_y * (iz + 1 == num_grids_z ? 1 - num_grids_z : 1);

						if (grids_is_full[i_next_x]) {
							grids_cut[index].x = 1.0 + grids_back[i_next_x].x;							
						}
						if (grids_is_full[i_next_y]) {
							grids_cut[index].y = 1.0 + grids_back[i_next_y].y;
						}
						if (grids_is_full[i_next_z]) {
							grids_cut[index].z = 1.0 + grids_back[i_next_z].z;
						}
					}
				}
			}
		}


		std::vector<vec3d> polygon;
		for (int iz = cutoff_edge_z; iz < num_grids_z - cutoff_edge_z; ++iz) {
			for (int iy = 0; iy < num_grids_y; ++iy) {
				for (int ix = 0; ix < num_grids_x; ++ix) {
					int grid_ids[8];
					grid_ids[0] = ix + num_grids_x * (iy + num_grids_y * iz);
					grid_ids[1] = grid_ids[0] + (ix + 1 == num_grids_x ? 1 - num_grids_x : 1);
					const int dy = num_grids_x * (iy + 1 == num_grids_y ? 1 - num_grids_y : 1);
					grid_ids[2] = grid_ids[0] + dy;
					grid_ids[3] = grid_ids[1] + dy;
					const int dz = num_grids_x * num_grids_y * (iz + 1 == num_grids_z ? 1 - num_grids_z : 1);
					grid_ids[4] = grid_ids[0] + dz;
					grid_ids[5] = grid_ids[1] + dz;
					grid_ids[6] = grid_ids[2] + dz;
					grid_ids[7] = grid_ids[3] + dz;

					bool v_is_full[8];
					for (int i = 0; i < 8; ++i) {
						v_is_full[i] = grids_is_full[grid_ids[i]];
					}


					vec3d cell_position{ (double)ix, (double)iy, (double)iz };
					mPolygonInUnitcell(grid_ids, v_is_full, grids_cut, cell_position, polygon);
				}
			}
		}

		//polygon of boundary//
		for (int iz = cutoff_edge_z; iz < num_grids_z - cutoff_edge_z; ++iz) {
			for (int iy = 0; iy < num_grids_y; ++iy) {

				const int ix = 0;
				int grid_ids[8];
				grid_ids[0] = ix + num_grids_x * (iy + num_grids_y * iz);
				grid_ids[1] = grid_ids[0] + (ix + 1 == num_grids_x ? 1 - num_grids_x : 1);
				const int dy = num_grids_x * (iy + 1 == num_grids_y ? 1 - num_grids_y : 1);
				grid_ids[2] = grid_ids[0] + dy;
				grid_ids[3] = grid_ids[1] + dy;
				const int dz = num_grids_x * num_grids_y * (iz + 1 == num_grids_z ? 1 - num_grids_z : 1);
				grid_ids[4] = grid_ids[0] + dz;
				grid_ids[5] = grid_ids[1] + dz;
				grid_ids[6] = grid_ids[2] + dz;
				grid_ids[7] = grid_ids[3] + dz;

				for (auto&& i : { 0,2,4,6 }) {
					grid_ids[i + 1] = grid_ids[i];
				}

				{
					int8_t vertex[4]{ 0,4,6,2 };

					bool v_is_full[4];
					for (int i = 0; i < 4; ++i) {
						v_is_full[i] = grids_is_full[grid_ids[vertex[i]]];
					}

					vec3d cell_position{ (double)ix, (double)iy, (double)iz };
					mPolygonBoundary(vertex, v_is_full, grid_ids, grids_cut, cell_position, polygon);
				}
				{
					int8_t vertex[4]{ 1,3,7,5 };

					bool v_is_full[4];
					for (int i = 0; i < 4; ++i) {
						v_is_full[i] = grids_is_full[grid_ids[vertex[i]]];
					}

					vec3d cell_position{ (double)(num_grids_x - 1), (double)iy, (double)iz };
					mPolygonBoundary(vertex, v_is_full, grid_ids, grids_cut, cell_position, polygon);
				}
			}
		}

		for (int iz = cutoff_edge_z; iz < num_grids_z - cutoff_edge_z; ++iz) {
			for (int ix = 0; ix < num_grids_x; ++ix) {
				const int iy = 0;
				int grid_ids[8];
				grid_ids[0] = ix + num_grids_x * (iy + num_grids_y * iz);
				grid_ids[1] = grid_ids[0] + (ix + 1 == num_grids_x ? 1 - num_grids_x : 1);
				const int dy = num_grids_x * (iy + 1 == num_grids_y ? 1 - num_grids_y : 1);
				grid_ids[2] = grid_ids[0] + dy;
				grid_ids[3] = grid_ids[1] + dy;
				const int dz = num_grids_x * num_grids_y * (iz + 1 == num_grids_z ? 1 - num_grids_z : 1);
				grid_ids[4] = grid_ids[0] + dz;
				grid_ids[5] = grid_ids[1] + dz;
				grid_ids[6] = grid_ids[2] + dz;
				grid_ids[7] = grid_ids[3] + dz;

				for (auto&& i : { 0,1,4,5 }) {
					grid_ids[i + 2] = grid_ids[i];
				}

				{
					int8_t vertex[4]{ 0,1,5,4 };

					bool v_is_full[4];
					for (int i = 0; i < 4; ++i) {
						v_is_full[i] = grids_is_full[grid_ids[vertex[i]]];
					}

					vec3d cell_position{ (double)ix, (double)iy, (double)iz };
					mPolygonBoundary(vertex, v_is_full, grid_ids, grids_cut, cell_position, polygon);
				}
				{
					int8_t vertex[4]{ 2,6,7,3 };

					bool v_is_full[4];
					for (int i = 0; i < 4; ++i) {
						v_is_full[i] = grids_is_full[grid_ids[vertex[i]]];
					}

					vec3d cell_position{ (double)ix, (double)(num_grids_y - 1), (double)iz };
					mPolygonBoundary(vertex, v_is_full, grid_ids, grids_cut, cell_position, polygon);
				}
			}
		}

		for (int iy = 0; iy < num_grids_y; ++iy) {
			for (int ix = 0; ix < num_grids_x; ++ix) {
				const int iz = cutoff_edge_z;
				const int iz_top = num_grids_z - cutoff_edge_z;
				int grid_ids[8];
				grid_ids[0] = ix + num_grids_x * (iy + num_grids_y * iz);
				grid_ids[1] = grid_ids[0] + (ix + 1 == num_grids_x ? 1 - num_grids_x : 1);
				const int dy = num_grids_x * (iy + 1 == num_grids_y ? 1 - num_grids_y : 1);
				grid_ids[2] = grid_ids[0] + dy;
				grid_ids[3] = grid_ids[1] + dy;

				grid_ids[4] = ix + num_grids_x * (iy + num_grids_y * mFolding(iz_top, num_grids_z));
				grid_ids[5] = grid_ids[4] + (ix + 1 == num_grids_x ? 1 - num_grids_x : 1);
				grid_ids[6] = grid_ids[4] + dy;
				grid_ids[7] = grid_ids[5] + dy;

				
				{
					int8_t vertex[4]{ 0,2,3,1 };

					bool v_is_full[4];
					for (int i = 0; i < 4; ++i) {
						v_is_full[i] = grids_is_full[grid_ids[vertex[i]]];
					}

					vec3d cell_position{ (double)ix, (double)iy, (double)iz };
					mPolygonBoundary(vertex, v_is_full, grid_ids, grids_cut, cell_position, polygon);
				}
				{
					int8_t vertex[4]{ 4,5,7,6 };

					bool v_is_full[4];
					for (int i = 0; i < 4; ++i) {
						v_is_full[i] = grids_is_full[grid_ids[vertex[i]]];
					}

					vec3d cell_position{ (double)ix, (double)iy, (double)(iz_top-1) };
					mPolygonBoundary(vertex, v_is_full, grid_ids, grids_cut, cell_position, polygon);
				}
			}
		}
		

		delete[] grids_is_full;
		delete[] grids_cut;
		delete[] grids_back;


		const vec3d s{ grid_width_x * scale, grid_width_y * scale, grid_width_z * scale };
		for (auto&& a : polygon) {
			a.z -= (double)cutoff_edge_z;
			a.x *= s.x;
			a.y *= s.y;
			a.z *= s.z;
		}

#if 1

		constexpr const char HEADER[80]{ "STL object file created by AIScope" };
		fwrite(HEADER, 80, 1, m_fp);
		
		//number of polygon
		const uint32_t num_polygon = (polygon.size() / 3);
		fwrite(&num_polygon, 4, 1, m_fp);
		

		for (int k = 0; k < num_polygon * 3; k += 3) {
			vec3d normal = Cross(polygon[k + 1] - polygon[k], polygon[k + 2] - polygon[k]);
			normal.Normalize();

			vec3f out[5]{ normal, polygon[k], polygon[k + 1], polygon[k + 2] };//cast//
			fwrite(&out, 12*4 + 2, 1, m_fp);
			//const uint16_t dummy{ 0 };
			//fwrite(&dummy, 2, 1, m_fp);
			
		}
#else
		constexpr const char HEADER[80]{ "STL object file created by AIScope" };
		fprintf(m_fp, "%s\n", HEADER);

		//number of polygon
		const uint32_t num_polygon = (polygon.size() / 3);
		
		fprintf(m_fp, "num_polygon=%d\n", num_polygon);

		
		for (int k = 0; k < num_polygon*3; k += 3) {
			vec3d normal = Cross(polygon[k + 1] - polygon[k], polygon[k + 2] - polygon[k]);
			normal.Normalize();
			
			fprintf(m_fp, "normal= %f, %f, %f\n", normal.x, normal.y, normal.z);
			fprintf(m_fp, "polygon0= %f, %f, %f\n", polygon[k].x, polygon[k].y, polygon[k].z);
			fprintf(m_fp, "polygon1= %f, %f, %f\n", polygon[k + 1].x, polygon[k + 1].y, polygon[k + 1].z);
			fprintf(m_fp, "polygon2= %f, %f, %f\n", polygon[k + 2].x, polygon[k + 2].y, polygon[k + 2].z);
		}
#endif

		return count;

	}
#if 0
private:
	int mFolding(int x, int num_grid) {
		while (x < 0) {
			x += num_grid;
		}
		while (x >= num_grid) {
			x -= num_grid;
		}
		return x;
	}

	struct Line
	{
		std::tuple<int8_t, int8_t> begin;
		std::tuple<int8_t, int8_t> end;
	};

	//4点で構成された四角形面において、断面と交わる線分をlinesに追加//
	void mSeLineInSurface(const int8_t vertexes[4], const bool is_full_list[8], std::list<Line>& lines) {

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
							begin_line = { vertexes[k], vertexes[next] };
						}
					} else {
						if (!v_is_full[next]) {
							start_is_found = false;
							lines.push_back({ begin_line,{ vertexes[k], vertexes[next] } });
						}
					}
				}
			}
		}

	}

	vec3d mPositionOnLine(std::tuple<int8_t, int8_t> pos, const int grid_ids[8], const vec3d* grid_cut) {
		const int id = std::min<int>(std::get<0>(pos), std::get<1>(pos));
		const int i = grid_ids[id];
		switch (std::get<0>(pos) ^ std::get<1>(pos)) {
		case 1:
			return { grid_cut[i].x, 0.0, 0.0 };
		case 2:
			return { 0.0, grid_cut[i].y, 0.0 };
		case 4:
			return { 0.0, 0.0, grid_cut[i].z };
		}
	}

	int mPolygonInUnitcell(const int grid_ids[8], const bool* is_full_list, const vec3d* grid_cut, const vec3d& cell_org, std::vector<vec3d>& poly_list) {

		std::list<Line> lines;


		const bool v_is_full[8]{ grid_ids[0], grid_ids[1], grid_ids[2], grid_ids[3], grid_ids[4], grid_ids[5], grid_ids[6], grid_ids[7] };

		//6つの面に断面が交差した際の線分をlinesに記録//
		{
			const int8_t vertexes[4] = { 0,1,3,2 };//surface 1//
			mSeLineInSurface(vertexes, is_full_list, lines);
		}
		{
			const int8_t vertexes[4] = { 0,1,5,4 };
			mSeLineInSurface(vertexes, is_full_list, lines);
		}
		{
			const int8_t vertexes[4] = { 0,4,6,2 };
			mSeLineInSurface(vertexes, is_full_list, lines);
		}
		{
			const int8_t vertexes[4] = { 7,5,4,6 };
			mSeLineInSurface(vertexes, is_full_list, lines);
		}
		{
			const int8_t vertexes[4] = { 7,5,1,3 };
			mSeLineInSurface(vertexes, is_full_list, lines);
		}
		{
			const int8_t vertexes[4] = { 7,3,2,6 };
			mSeLineInSurface(vertexes, is_full_list, lines);
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
						poly.splice(std::prev(poly.end()), lines, it);
						break;
					}
				}
			}
			//ここに来たら閉じたlinesになっているはず//
			//ポリゴンを追加//
			auto first_pos = poly.front().begin;
			for (auto it = std::next(poly.begin()); it != poly.end(); it++) {
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
};
#endif

