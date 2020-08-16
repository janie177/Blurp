#pragma once

namespace blurp
{
    enum class LockType
    {
        READ,   //Lock for reading. Multiple reads can happen at a time.
        WRITE   //Lock for writing. Only one thread can write and no reading is possible.
    };
}