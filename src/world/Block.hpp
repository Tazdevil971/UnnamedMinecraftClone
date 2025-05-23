#pragma once

enum class Block {
    AIR = 0,
    GRASS,
    DIRT,
    COBBLESTONE,
    WOOD_LOG,
    LEAF,
    CHERRY_LEAF,
    DIAMOND
};

enum class Side {
    SIDE_Y_POS,
    SIDE_Z_POS,
    SIDE_Z_NEG,
    SIDE_X_POS,
    SIDE_X_NEG,
    SIDE_Y_NEG
};