#pragma once
#include <cstdint>
#include <string>
#include <vector>

class PGMImage {
public:
	PGMImage(std::string fileName);
	PGMImage(uint16_t width, uint16_t height) : m_width(width), m_height(height), m_data(width* height) {}

	bool Write(std::string fileName);

	uint16_t width() const { return m_width; }
	uint16_t height() const { return m_height; }

	uint16_t* data() { if (m_width && m_height) { return m_data.data(); } else { return nullptr; } }

	uint16_t& operator()(uint16_t x, uint16_t y) { return m_data.data()[y * m_width + x]; }

protected:
	uint16_t m_width = 0;
	uint16_t m_height = 0;
	std::vector<uint16_t> m_data;
};
