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

#include <iostream>
#include <vector>
#include <math.h>

#include "pgm.hpp"

#define THICKNESS 0.05

void vertex(std::ostream &ofs, float x, float y, float z, unsigned &nVerts) {
  ofs << "v " << x << " " << y << " " << z << std::endl;
  ++nVerts;
}

void triangle(std::ostream &ofs, int a, int b, int c, unsigned nTris) {
  ofs << "f " << (a + 1) << " " << (b + 1) << " " << (c + 1) << std::endl;
  ++nTris;
}

void quad(std::ostream &ofs, unsigned a, unsigned b, unsigned c, unsigned d, unsigned nTris) {
  triangle(ofs, a, b, c, nTris);
  triangle(ofs, c, d, a, nTris);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: pgmtoobj <input.ppm>" << std::endl;
    return 0;
  }

  std::vector<unsigned char> data;
  unsigned w, h;
  if (!PnmReader::read(argv[1], w, h, data, &std::cerr)) {
    return 0;
  }
  std::cerr << "image size is " << w << "x" << h << std::endl;

  std::ofstream vfs ( "tmp_verts.txt" , std::ofstream::out );
  if(!vfs.good()) {
    std::cerr << "Cannot open tmp file tmp_verts.txt for writing" << std::endl;
    return 0;
  }
  std::ofstream tfs ( "tmp_tris.txt" , std::ofstream::out );
  if(!tfs.good()) {
    std::cerr << "Cannot open tmp file tmp_tris.txt for writing" << std::endl;
    return 0;
  }

  unsigned nVerts = 0;
  unsigned nTris = 0;

  //write grid of vertices
  for (unsigned y = 0; y < h + 1; ++y) {
    for (unsigned x = 0; x < w + 1; ++x) {
      vertex(vfs, x / (float)w       , y / (float)h      ,  THICKNESS, nVerts);
    }
  }
  for (unsigned y = 0; y < h + 1; ++y) {
    for (unsigned x = 0; x < w + 1; ++x) {
      vertex(vfs, x / (float)w       , y / (float)h      ,  -THICKNESS, nVerts);
    }
  }

  //write quads
  for (unsigned y = 0; y < h; ++y) {
    for (unsigned x = 0; x < w; ++x) {
      unsigned a = (w+1) * y       + x;
      unsigned b = (w+1) * (y + 1) + x;
      unsigned c = (w+1) * (y + 1) + x + 1;
      unsigned d = (w+1) * y       + x + 1;
      if (data[3 * (w * y + x)] < 128) {
        quad(tfs, a, b, c, d, nTris);
      }
      if (data[3 * (w * y + x) + 1] < 128) {
        quad(tfs, a + (w+1)*(h+1), b + (w+1)*(h+1), c + (w+1)*(h+1), d + (w+1)*(h+1), nTris);
      }
    }
  }

  vfs.close();
  tfs.close();

  {
    char buf[256];
    //std::cout << "#clement courbet" << std::endl << nVerts << " " << nTris << std::endl;
    std::ifstream vfs ( "tmp_verts.txt" , std::ifstream::in );
    if(!vfs.good()) {
      std::cerr << "Cannot open tmp file tmp_verts.txt for writing" << std::endl;
      return 0;
    }
    while (vfs.getline(buf, 256)) {
      std::cout << buf << std::endl;
    }
    vfs.close();
    std::ifstream tfs ( "tmp_tris.txt" , std::ifstream::in );
    if(!tfs.good()) {
      std::cerr << "Cannot open tmp file tmp_tris.txt for writing" << std::endl;
      return 0;
    }
    while (tfs.getline(buf, 256)) {
      std::cout << buf << std::endl;
    }
    tfs.close();
  }


  return 0;
}



