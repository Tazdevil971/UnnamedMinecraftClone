#pragma once

#include "Context.hpp"
#include "BufferManager.hpp"
#include "Mesh.hpp"

namespace render {

class TextureManager {
public:
    TextureManager(std::shared_ptr<Context> ctx, std::shared_ptr<BufferManager> bufferMgr);

    SimpleTexture createSimpleTexture(/* ... */);

    void deallocateSimpleTexture(SimpleTexture texture);
};

}