#pragma once

#include <memory>

#include "render/BufferManager.hpp"
#include "render/Context.hpp"
#include "render/Renderer.hpp"
#include "render/TextureManager.hpp"
#include "render/Window.hpp"

class PlayerController : public render::Window {
   public:
    PlayerController();

    void cleanup();

   protected:
    void onFrame() override;
    void onResize(int width, int height) override;

   private:
    bool windowResized{false};

    std::shared_ptr<render::Context> ctx;
    std::shared_ptr<render::Swapchain> swapchain;
    std::shared_ptr<render::BufferManager> bufferMgr;
    std::shared_ptr<render::TextureManager> textureMgr;
    std::shared_ptr<render::Renderer> renderer;

    render::SimpleModel model;
};