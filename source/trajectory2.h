#pragma once
#include <map>
#include "trajectory.h"
#include "vec3.h"

class Trajectory2
{
private:
	std::map<int, vec3f> m_buffer;
	int m_buf_sz;

	std::map<int, vec3f>::iterator m_itr;
public:

	Trajectory2() : m_buf_sz(0) {};
	//virtual ~Trajectory2();

	bool Add(int time_frame, vec3f& position) {
		auto res = m_buffer.insert(std::pair<int, vec3d>(time_frame, position));
		m_buf_sz++;
		return true;
	}

	int GetSize() const {
		return m_buf_sz;
	}


	int GetFirst(vec3f* position) {
		m_itr = m_buffer.begin();
		*position = m_itr->second;
		return m_itr->first;
	}

	int GetNext(vec3f* position) {
		if (m_itr == m_buffer.end()) {
			return -1;
		}
		++m_itr;
		if (m_itr == m_buffer.end()) {
			return -1;
		}

		*position = m_itr->second;
		return m_itr->first;
	}

	int GetCurrent(vec3f* position) {
		*position = m_itr->second;
		return m_itr->first;
	}

	//void Set(const TRAJECTORY_POS* buf, int count);
};

