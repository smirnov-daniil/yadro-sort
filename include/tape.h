#pragma once
#ifndef __tape_h_
#define __tape_h_

#include <cstdint>

class tape {
public:
  virtual bool read(std::int32_t& value) = 0;

  virtual bool write(std::int32_t value) = 0;

  virtual bool step(bool forward = true) = 0;

  virtual void rewind() = 0;

  virtual ~tape() = default;
};

#endif // !__tape_h_
