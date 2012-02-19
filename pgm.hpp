/**
 * Copyright (C) 2012 Clement Courbet
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PGMRW_HPP_
#define PGMRW_HPP_

#include <fstream>
#include <ostream>

class PnmReader {
  public:
  static bool read(const char * filename, unsigned &w, unsigned &h, std::vector<unsigned char> &data, std::ostream *err = NULL) {
    std::ifstream ifs ( filename , std::ifstream::in );
    if(!ifs.good()) {
      if (err) {
        *err << "Cannot open file '" << filename << "' for reading." << std::endl;
      }
      return false;
    }

    if (!read(ifs, w, h, data, err)) {
      return false;
    }

    ifs.close();
    return true;
  }

  static bool read(std::ifstream& ifs, unsigned &w, unsigned &h, std::vector<unsigned char> &data, std::ostream *err = NULL) {
    data.resize(0);
    char buf[bufSize];
    ifs.getline(buf, bufSize);
    if (buf[0] != 'P') {
      if (err) {
        *err << "Not a PNM file." << std::endl;
      }
      return false;
    }
    switch (buf[1]) {
      case '1':
        return _read<AsciiPBM>(ifs, w, h, data, err);
      case '2':
        return _read<AsciiPGM>(ifs, w, h, data, err);
      case '3':
        return _read<AsciiPPM>(ifs, w, h, data, err);
      case '4':
        return _read<BinPGM>(ifs, w, h, data, err); //FIXME
      case '5':
        return _read<BinPGM>(ifs, w, h, data, err);
      case '6':
        return _read<BinPPM>(ifs, w, h, data, err);
      default:
        if (err) {
          *err << "Not a PNM file." << std::endl;
        }
        return false;
    }
  }

  private:
    enum PixType {AsciiPBM, AsciiPGM, AsciiPPM, BinPBM, BinPGM, BinPPM};

  static bool _readCommentWidthHeight(std::ifstream& ifs, unsigned &width, unsigned &height, std::ostream *err) {
    //read comments
    char buf[bufSize];
    while (ifs.peek() == '#') {
      ifs.getline(buf, bufSize);
    }
    ifs >> width;
    if (ifs.eof() || ifs.fail()) {
      if (err) {
        *err << "Cannot read PNM width." << std::endl;
      }
      return false;
    }
    ifs >> height;
    if (ifs.eof() || ifs.fail()) {
      if (err) {
        *err << "Cannot read PNM width." << std::endl;
      }
      return false;
    }
    unsigned depth;
    ifs >> depth;
    if (ifs.eof() || ifs.fail()) {
      if (err) {
        *err << "Cannot read PNM depth." << std::endl;
      }
      return false;
    }
    ifs.getline(buf, bufSize); //read the remaining of the line (\n)
    return true;
  }

  //binary RGB
  template<PixType type>
  static bool _read(std::ifstream& ifs, unsigned &w, unsigned &h, std::vector<unsigned char> &data, std::ostream *err = NULL) {
    if (!_readCommentWidthHeight(ifs, w, h, err)) {
      return false;
    }
    data.reserve(w * h * 3);
    for (unsigned i = 0; i < w * h; ++i) {
      _readPixel<type>(ifs, data);
    }
    return true;
  }

  template<PixType type>
  static void _readPixel(std::ifstream& ifs, std::vector<unsigned char> &data);

  static const size_t bufSize = 512;
};

template <>
void PnmReader::_readPixel<PnmReader::AsciiPBM>(std::ifstream& ifs, std::vector<unsigned char> &data) {
  unsigned v;
  ifs >> v;
  data.push_back(v ? 255 : 0);
  data.push_back(v ? 255 : 0);
  data.push_back(v ? 255 : 0);
}

template <>
void PnmReader::_readPixel<PnmReader::AsciiPGM>(std::ifstream& ifs, std::vector<unsigned char> &data) {
  unsigned v;
  ifs >> v;
  data.push_back((unsigned char)v);
  data.push_back((unsigned char)v);
  data.push_back((unsigned char)v);
}

template <>
void PnmReader::_readPixel<PnmReader::AsciiPPM>(std::ifstream& ifs, std::vector<unsigned char> &data) {
  unsigned v;
  ifs >> v;
  data.push_back((unsigned char)v);
  ifs >> v;
  data.push_back((unsigned char)v);
  ifs >> v;
  data.push_back((unsigned char)v);
}

template <>
void PnmReader::_readPixel<PnmReader::BinPGM>(std::ifstream& ifs, std::vector<unsigned char> &data) {
  unsigned char v;
  ifs.read((char*)&v, 1);
  data.push_back(v);
  data.push_back(v);
  data.push_back(v);
}

template <>
void PnmReader::_readPixel<PnmReader::BinPPM>(std::ifstream& ifs, std::vector<unsigned char> &data) {
  unsigned char v;
  ifs.read((char*)&v, 1);
  data.push_back(v);
  ifs.read((char*)&v, 1);
  data.push_back(v);
  ifs.read((char*)&v, 1);
  data.push_back(v);
}

class PpmWriter {
  public:
  static bool write(const char * filename, unsigned w, unsigned h, const std::vector<unsigned char> &data, std::ostream *err = NULL) {
    std::ofstream ofs ( filename , std::ifstream::out );
    if(!ofs.good()) {
      if (err) {
        *err << "Cannot open file '" << filename << "' for writing." << std::endl;
      }
      return false;
    }

    if (!write(ofs, w, h, data, err)) {
      return false;
    }

    ofs.close();
    return true;
  }

  static bool write(std::ofstream& ofs, unsigned w, unsigned h, const std::vector<unsigned char> &data, std::ostream *err = NULL) {
    if (data.size() != w * h * 3) {
      if (err) {
        *err << "PpmWriter: Data has incorrect size (" << data.size() << " but image size is " << w << "x" << h << ")." << std::endl;
      }
      return false;
    }
    ofs << "P6\n" << w << " " << h << "\n255\n";
    for (size_t i=0; i<data.size(); ++i) {
      ofs.write((const char*)&data[i], 1);
    }
    return true;
  }

};


#endif