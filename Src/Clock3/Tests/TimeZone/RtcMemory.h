#ifndef RTC_MEMORY_H
#define RTC_MEMORY_H

#include <RTClib.h>

class RtcMemory
{
  public:
    const bool begin()
    {
      return true;
    }

    DateTime now()
    {
      return _now;
    }

    void adjust(DateTime dt)
    {
      this->_now = dt;
    }

  protected:
    DateTime _now;
};
#endif
