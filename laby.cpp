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

#include <queue>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <map>
#include <limits>
#include <sstream>
#include <math.h>

#include <unordered_map>

#include "pgm.hpp"

struct Node {
  Node (size_t topPos, size_t bottomPos, unsigned time): topPos(topPos), bottomPos(bottomPos), time(time) {
  }

  size_t topPos;
  size_t bottomPos;
  unsigned time;

  bool operator <(const Node& other) const {
    return time > other.time;
  }

  private:
    Node();
};

class Ring {
  public:
    Ring(double interPinDistance, double diameter, double tolerance)
     : _interPinDistance(interPinDistance), _diameter(diameter), _tolerance(tolerance) {
    }

    template <class LabyT>
    bool validatePinPos(double topX, double topY, double bottomX, double bottomY, const LabyT& laby) const {
      if ( (topX - bottomX)*(topX - bottomX) + (topY - bottomY)*(topY - bottomY) < (_interPinDistance + _tolerance) * (_interPinDistance + _tolerance) &&
             (topX - bottomX)*(topX - bottomX) + (topY - bottomY)*(topY - bottomY) > (_interPinDistance - _tolerance) * (_interPinDistance - _tolerance)) {
        //first check OK, now check that the ring does not intersect the laby.
        double vx = (topX - bottomX) / _interPinDistance;
        double vy = (topY - bottomY) / _interPinDistance;
        int x = (unsigned)(bottomX + vx * _diameter + 0.5);
        int y = (unsigned)(bottomY + vy * _diameter + 0.5);
        if (x >= 0 && x < (int)laby.getWidth() && y >= 0 && y < (int)laby.getHeight()) {
          return laby.atTop(laby.coordsToPos(x, y)) != LabyT::Path && laby.atBottom(laby.coordsToPos(x, y)) != LabyT::Path;
        } else {
          return true;
        }
      } else {
        return false;
      }
    }

    double getPinDistance() const {
      return _interPinDistance;
    }

    double getDiameter() const {
      return _diameter;
    }

  private:
    double _interPinDistance;
    double _diameter;
    double _tolerance;

    Ring();
};

/**
 * The labyrinth;
 *
 */
struct Laby {
public:
  enum CellType { Path = 255, Wall = 0, Exit=254};
  static const size_t InvalidPos = 0xffffffff;

  Laby(unsigned width, unsigned height): _w(width), _h(height) {
    _topMap.resize(_w*_h, Path);
    _bottomMap.resize(_w*_h, Path);
  }

  //create from pnm
  Laby(const char* filename, bool switchTopBottom) {
    //read PGM
    std::vector<unsigned char> data;
    if (PnmReader::read(filename, _w, _h, data, &std::cerr)) {
      _topMap.resize(_w*_h, Path);
      _bottomMap.resize(_w*_h, Path);
      if (switchTopBottom) {
        for (unsigned i = 0; i < _w*_h; ++i) {
          //red is bottom, green is top
          // >128 is path, <128 is wall
          // blue=255 is exit
          if (data[3*i] > 128) {
            _bottomMap[i] = Path;
          } else {
            _bottomMap[i] = Wall;
          }
          if (data[3*i + 1] > 128) {
            _topMap[i] = Path;
          } else {
            _topMap[i] = Wall;
          }
          if (data[3*i + 2] == 255) {
            _bottomMap[i] = Exit;
            _topMap[i] = Exit;
          }
        }
      } else {
        for (unsigned i = 0; i < _w*_h; ++i) {
          //red is top, green is bottom
          // >128 is path, <128 is wall
          // blue=255 is exit
          if (data[3*i] > 128) {
            _topMap[i] = Path;
          } else {
            _topMap[i] = Wall;
          }
          if (data[3*i + 1] > 128) {
            _bottomMap[i] = Path;
          } else {
            _bottomMap[i] = Wall;
          }
          if (data[3*i + 2] == 255) {
            _bottomMap[i] = Exit;
            _topMap[i] = Exit;
          }
        }
      }
    } else {
      _w = 0;
      _h = 0;
    }
  }

  unsigned getWidth() const {
    return _w;
  }

  unsigned getHeight() const {
    return _h;
  }

  CellType atTop(size_t pos) const {
    return _bottomMap[pos];
  }

  CellType atBottom(size_t pos) const {
    return _topMap[pos];
  }

  size_t coordsToPos(unsigned x, unsigned y) const {
    assert(y < _h);
    assert(x < _w);
    return _w*y + x;
  }

  void posToCoords(size_t pos, unsigned &x, unsigned &y) const {
    assert(pos != InvalidPos);
    x = pos % _w;
    y = pos / _w;
  }

  size_t left(size_t pos) const {
    if (pos == InvalidPos) {
      return InvalidPos;
    }
    unsigned x, y;
    posToCoords(pos, x, y);
    if (x > 0) {
      return coordsToPos(x - 1, y);
    } else {
      return InvalidPos;
    }
  }

  size_t right(size_t pos) const {
    if (pos == InvalidPos) {
      return InvalidPos;
    }
    unsigned x, y;
    posToCoords(pos, x, y);
    if (x < _w - 1) {
      return coordsToPos(x + 1, y);
    } else {
      return InvalidPos;
    }
  }

  size_t up(size_t pos) const {
    if (pos == InvalidPos) {
      return InvalidPos;
    }
    unsigned x, y;
    posToCoords(pos, x, y);
    if (y > 0) {
      return coordsToPos(x, y - 1);
    } else  {
      return InvalidPos;
    }
  }

  size_t down(size_t pos) const {
    if (pos == InvalidPos) {
      return InvalidPos;
    }
    unsigned x, y;
    posToCoords(pos, x, y);
    if (y < _h - 1) {
      return coordsToPos(x, y + 1);
    } else  {
      return InvalidPos;
    }
  }

  template <class Ring>
  bool validate(size_t topPos, size_t bottomPos, Ring& ring) const {
    assert(topPos != InvalidPos);
    assert(bottomPos != InvalidPos);
    if (_topMap[topPos] == Wall || _bottomMap[bottomPos] == Wall) {
      //positions must be in a path
      return false;
    } else {
      unsigned xTop, xBottom, yTop, yBottom;
      posToCoords(topPos, xTop, yTop);
      posToCoords(bottomPos, xBottom, yBottom);
      return ring.validatePinPos(xTop, yTop, xBottom, yBottom, *this);
    }
  }

  void draw(std::vector<unsigned char> &data, size_t topPos, size_t bottomPos) const {
    data.resize(0);
    data.reserve(_w*_h*3);
    for (unsigned i = 0; i < _w*_h; ++i) {
      data.push_back(_topMap[i]); //R
      data.push_back(_bottomMap[i]); //G
      data.push_back((_topMap[i] == Exit)*255); //B
    }
    data[3 * topPos] = 0;
    data[3 * topPos + 1] = 0;
    data[3 * topPos + 2] = 255;
    data[3 * bottomPos] = 0;
    data[3 * bottomPos + 1] = 0;
    data[3 * bottomPos + 2] = 255;
  }


private:

  double dist();

  unsigned _w;
  unsigned _h;
  std::vector<CellType> _topMap;
  std::vector<CellType> _bottomMap;
};

struct VisitedPos {
    VisitedPos(): time(0xffffffff), prevObjOffset(std::numeric_limits<size_t>::max()) {
    }
    unsigned time;
    size_t prevObjOffset; //'pointer' to previous visited pos
};

struct VisitedPositionsHashMap {
  VisitedPositionsHashMap(unsigned size): _size(size) {
  }

  bool operator() (size_t topPos, size_t bottomPos, unsigned time) const {
    std::unordered_map<size_t, VisitedPos>::const_iterator it = _visited.find(_size*topPos + bottomPos);
    return it != _visited.end() && it->second.time <= time;
  }

  void set(size_t topPos, size_t bottomPos, unsigned time, size_t prevTopPos, size_t prevBottomPos) {
    assert(!(*this)(topPos, bottomPos, time));
    _visited[_size*topPos + bottomPos].time = time;
    _visited[_size*topPos + bottomPos].prevObjOffset = _size*prevTopPos + prevBottomPos;
  }

  void setOrigin(size_t topPos, size_t bottomPos) {
    _visited[_size*topPos + bottomPos].time = 0;
    _visited[_size*topPos + bottomPos].prevObjOffset = std::numeric_limits<size_t>::max();
  }

  size_t offsetOf(size_t topPos, size_t bottomPos) const {
    return _size*topPos + bottomPos;
  }

  void posOf(size_t offset, size_t &topPos, size_t &bottomPos) const {
    topPos = offset / _size;
    bottomPos = offset - topPos * _size;
  }

  const VisitedPos& atOffset(size_t offset) const {
    std::unordered_map<size_t, VisitedPos>::const_iterator it = _visited.find(offset);
    assert(it != _visited.end());
    return it->second;
  }

  private:
    std::unordered_map<size_t, VisitedPos> _visited;
    size_t _size;
};

template <class VisitedPositions>
void backtrackToStart(const Laby& laby, const Ring& ring, const VisitedPositions& beenThere, size_t topPos, size_t bottomPos) {
  std::vector<unsigned char> data;
  for(size_t offset = beenThere.offsetOf(topPos, bottomPos); offset != std::numeric_limits<size_t>::max(); ) {
    const VisitedPos &vp = beenThere.atOffset(offset);
    size_t tPos, bPos;
    beenThere.posOf(offset, tPos, bPos);
    {
      unsigned xt, yt, xb, yb;
      laby.posToCoords(tPos, xt, yt);
      laby.posToCoords(bPos, xb, yb);
      std::cout << vp.time;
      //write center and angle.
      float vtbx = ((float)xt - (float)xb) / laby.getWidth();
      float vtby = ((float)yt - (float)yb) / laby.getHeight();
      float vNorm = sqrt(vtbx * vtbx + vtby * vtby);
      vtbx /= vNorm;
      vtby /= vNorm;
      std::cout << " " << (float)xt / laby.getWidth() << " " << (float)yt / laby.getHeight();
      std::cout << " " << (float)xb / laby.getWidth() << " " << (float)yb / laby.getHeight();
      float rx = (xb + vtbx * ring.getDiameter())/ (float)laby.getWidth() ;
      float ry = (yb + vtby * ring.getDiameter())/ (float)laby.getHeight() ;
      std::cout << " " << rx << " " << ry;
      std::cout << std::endl;
    }
    offset = vp.prevObjOffset;
  }
}

int main(int argc, char **argv) {
  std::priority_queue<Node> queue;

  if (argc < 4) {
    std::cerr << "Usage: laby <input.ppm> <pinDist> <diameter> [switch]" << std::endl;
    return 0;
  }

  double pinDist = atof(argv[2]);
  double diameter = atof(argv[3]);

  bool switchTB = false;
  if (argc > 4) {
    switchTB = true;
  }

  Laby laby(argv[1], switchTB);
  unsigned width = laby.getWidth();
  unsigned height = laby.getHeight();

  Ring ring(pinDist, diameter, sqrt(2.0)/2.0);
  VisitedPositionsHashMap beenThereBefore(width*height);
  size_t startTopPos = laby.coordsToPos(0, 0);
  size_t startBottomPos = laby.coordsToPos(0, (unsigned)ring.getPinDistance());

  queue.push(Node(startTopPos, startBottomPos, 0));
  beenThereBefore.setOrigin(startTopPos, startBottomPos);


  unsigned lastTime = 0;
  while (!queue.empty()) {
    Node current = queue.top();
    queue.pop();
    if (lastTime != current.time) {
      lastTime = current.time;
      unsigned x, y;
      std::cerr << "time: " << current.time << " pos (" << current.topPos << " " << current.bottomPos << ") = ";
      laby.posToCoords(current.topPos, x, y);
      std::cerr << "(" << x << "," <<  y<< ")";
      laby.posToCoords(current.bottomPos, x, y);
      std::cerr << " (" << x << "," <<  y<< ")" << std::endl;
      std::cerr << "  nodes: " << queue.size() << std::endl;
    }
    if (laby.atTop(current.topPos) == Laby::Exit && laby.atBottom(current.bottomPos) == Laby::Exit) {
      std::cerr << "Found path in " << current.time << " steps" << std::endl;
      backtrackToStart(laby, ring, beenThereBefore, current.topPos, current.bottomPos);
      return 0;
    }
    /**
     * Try all 81 (l/s/r)*(t/s/b)*(l/s/r)*(t/s/b) possibilities,
     * For a possibility to be physically possible:
     *  (1) the labyrinth must be empty on both top and bottom
     *  (2) the pins must be separated by the correct distance
     *  (3) the ring must not wipe through something other than InputSpace. TODO
     */
#define TRY_POS(nextTopPos, nextBottomPos)\
    if ((nextTopPos) != Laby::InvalidPos && (nextBottomPos) != Laby::InvalidPos && laby.validate((nextTopPos), (nextBottomPos), ring) && !beenThereBefore((nextTopPos), (nextBottomPos), current.time + 1)) {\
      queue.push(Node((nextTopPos), (nextBottomPos), current.time + 1));\
      beenThereBefore.set((nextTopPos), (nextBottomPos), current.time + 1, current.topPos, current.bottomPos);\
    }
    TRY_POS(laby.up(laby.left(current.topPos)), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.up(laby.left(current.topPos)), laby.up(current.bottomPos))
    TRY_POS(laby.up(laby.left(current.topPos)), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.up(laby.left(current.topPos)), laby.left(current.bottomPos))
    TRY_POS(laby.up(laby.left(current.topPos)), current.bottomPos)
    TRY_POS(laby.up(laby.left(current.topPos)), laby.right(current.bottomPos))

    TRY_POS(laby.up(laby.left(current.topPos)), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.up(laby.left(current.topPos)), laby.down(current.bottomPos))
    TRY_POS(laby.up(laby.left(current.topPos)), laby.down(laby.right(current.bottomPos)))


    TRY_POS(laby.up(current.topPos), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.up(current.topPos), laby.up(current.bottomPos))
    TRY_POS(laby.up(current.topPos), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.up(current.topPos), laby.left(current.bottomPos))
    TRY_POS(laby.up(current.topPos), current.bottomPos)
    TRY_POS(laby.up(current.topPos), laby.right(current.bottomPos))

    TRY_POS(laby.up(current.topPos), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.up(current.topPos), laby.down(current.bottomPos))
    TRY_POS(laby.up(current.topPos), laby.down(laby.right(current.bottomPos)))


    TRY_POS(laby.up(laby.right(current.topPos)), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.up(laby.right(current.topPos)), laby.up(current.bottomPos))
    TRY_POS(laby.up(laby.right(current.topPos)), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.up(laby.right(current.topPos)), laby.left(current.bottomPos))
    TRY_POS(laby.up(laby.right(current.topPos)), current.bottomPos)
    TRY_POS(laby.up(laby.right(current.topPos)), laby.right(current.bottomPos))

    TRY_POS(laby.up(laby.right(current.topPos)), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.up(laby.right(current.topPos)), laby.down(current.bottomPos))
    TRY_POS(laby.up(laby.right(current.topPos)), laby.down(laby.right(current.bottomPos)))



    TRY_POS(laby.left(current.topPos), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.left(current.topPos), laby.up(current.bottomPos))
    TRY_POS(laby.left(current.topPos), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.left(current.topPos), laby.left(current.bottomPos))
    TRY_POS(laby.left(current.topPos), current.bottomPos)
    TRY_POS(laby.left(current.topPos), laby.right(current.bottomPos))

    TRY_POS(laby.left(current.topPos), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.left(current.topPos), laby.down(current.bottomPos))
    TRY_POS(laby.left(current.topPos), laby.down(laby.right(current.bottomPos)))


    TRY_POS(current.topPos, laby.up(laby.left(current.bottomPos)))
    TRY_POS(current.topPos, laby.up(current.bottomPos))
    TRY_POS(current.topPos, laby.up(laby.right(current.bottomPos)))

    TRY_POS(current.topPos, laby.left(current.bottomPos))
    //TRY_POS(current.topPos, current.bottomPos)
    TRY_POS(current.topPos, laby.right(current.bottomPos))

    TRY_POS(current.topPos, laby.down(laby.left(current.bottomPos)))
    TRY_POS(current.topPos, laby.down(current.bottomPos))
    TRY_POS(current.topPos, laby.down(laby.right(current.bottomPos)))


    TRY_POS(laby.right(current.topPos), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.right(current.topPos), laby.up(current.bottomPos));
    TRY_POS(laby.right(current.topPos), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.right(current.topPos), laby.left(current.bottomPos))
    TRY_POS(laby.right(current.topPos), current.bottomPos)
    TRY_POS(laby.right(current.topPos), laby.right(current.bottomPos))

    TRY_POS(laby.right(current.topPos), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.right(current.topPos), laby.down(current.bottomPos))
    TRY_POS(laby.right(current.topPos), laby.down(laby.right(current.bottomPos)))



    TRY_POS(laby.down(laby.left(current.topPos)), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.down(laby.left(current.topPos)), laby.up(current.bottomPos))
    TRY_POS(laby.down(laby.left(current.topPos)), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.down(laby.left(current.topPos)), laby.left(current.bottomPos))
    TRY_POS(laby.down(laby.left(current.topPos)), current.bottomPos)
    TRY_POS(laby.down(laby.left(current.topPos)), laby.right(current.bottomPos))

    TRY_POS(laby.down(laby.left(current.topPos)), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.down(laby.left(current.topPos)), laby.down(current.bottomPos))
    TRY_POS(laby.down(laby.left(current.topPos)), laby.down(laby.right(current.bottomPos)))


    TRY_POS(laby.down(current.topPos), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.down(current.topPos), laby.up(current.bottomPos))
    TRY_POS(laby.down(current.topPos), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.down(current.topPos), laby.left(current.bottomPos))
    TRY_POS(laby.down(current.topPos), current.bottomPos)
    TRY_POS(laby.down(current.topPos), laby.right(current.bottomPos))

    TRY_POS(laby.down(current.topPos), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.down(current.topPos), laby.down(current.bottomPos))
    TRY_POS(laby.down(current.topPos), laby.down(laby.right(current.bottomPos)))


    TRY_POS(laby.down(laby.right(current.topPos)), laby.up(laby.left(current.bottomPos)))
    TRY_POS(laby.down(laby.right(current.topPos)), laby.up(current.bottomPos))
    TRY_POS(laby.down(laby.right(current.topPos)), laby.up(laby.right(current.bottomPos)))

    TRY_POS(laby.down(laby.right(current.topPos)), laby.left(current.bottomPos))
    TRY_POS(laby.down(laby.right(current.topPos)), current.bottomPos)
    TRY_POS(laby.down(laby.right(current.topPos)), laby.right(current.bottomPos))

    TRY_POS(laby.down(laby.right(current.topPos)), laby.down(laby.left(current.bottomPos)))
    TRY_POS(laby.down(laby.right(current.topPos)), laby.down(current.bottomPos))
    TRY_POS(laby.down(laby.right(current.topPos)), laby.down(laby.right(current.bottomPos)))

  }

  std::cerr << "Path not found" << std::endl;
  return 0;
}



