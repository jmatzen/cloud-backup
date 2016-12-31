#pragma once

namespace jm
{
  class NonCopyable
  {
  public:
    NonCopyable() {}
  private:
    NonCopyable(NonCopyable&);
    void operator=(NonCopyable&);
  };
}