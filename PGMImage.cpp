#include "PGMImage.h"
#include <fstream>
#include <sstream>
#include <iostream>

static bool isBigEndian() {
  union {
    uint32_t i;
    char c[4];
  } testUnion = { 0x01020304 };

  return testUnion.c[0] == 1;
}

static void fixEndian(std::vector<uint16_t> &data) {
  if (!isBigEndian()) {
    for (uint16_t &value : data) {
      value = (value >> 8) | (value << 8);
    }
  }
}

PGMImage::PGMImage(std::string fileName)
{
  std::ifstream file(fileName, std::ios::binary);
  if (!file) {
    std::cerr << "PGMImage(): Could not open file: " + fileName << std::endl;
    return;
  }
  std::string line;

  // Read and check the header line
  std::getline(file, line);
  if (line != "P5") {
    std::cerr << "PGMImage(): File is not a PGM file: " + fileName << std::endl;
    return;
  }

  // Read and parse the width and height line
  std::getline(file, line);
  std::istringstream lineStream(line);
  lineStream >> m_width >> m_height;
  if (m_width == 0 || m_height == 0) {
    std::cerr << "PGMImage(): Invalid width or height in PGM file: " + fileName << std::endl;
    return;
  }

  // Read and check the maximum-value line, which should have 65535.
  std::getline(file, line);
  if (line != "65535") {
    std::cerr << "PGMImage(): Invalid maximum value in PGM file: " + fileName << std::endl;
    return;
  }

  // Read the specified number of 16-bit unsigned values into the data array, swapping endianness
  // if needed.
  m_data.resize(m_width * m_height);
  file.read(reinterpret_cast<char*>(m_data.data()), m_width * m_height * sizeof(uint16_t));
  fixEndian(m_data);
}

bool PGMImage::Write(std::string fileName)
{
  std::ofstream file(fileName, std::ios::binary);
  if (!file) {
    std::cerr << "PGMImage::Write(): Could not open file: " + fileName << std::endl;
    return false;
  }
  // Write the header lines to the file.
  file << "P5\n";
  file << m_width << " " << m_height << "\n";
  file << "65535\n";
  // Write the data to the file after swapping endianness if needed.
  std::vector<uint16_t> *dataToSend = &m_data;
  std::vector<uint16_t> dataCopy;
  if (!isBigEndian()) {
    dataCopy = m_data;
    fixEndian(dataCopy);
    dataToSend = &dataCopy;
  }
  file.write(reinterpret_cast<char*>(dataToSend->data()), m_width * m_height * sizeof(uint16_t));
  return true;
}
