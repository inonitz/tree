#pragma once
#include <tree/C/treelib_api.h>
#include <cstdint>


enum class TREELIB_NO_EXPORT AVLTreeRotationState : uint8_t {
    NONE       = 0,
    LEFTLEFT   = 1,
    RIGHTRIGHT = 2,
    LEFTRIGHT  = 3,
    RIGHTLEFT  = 4
};
