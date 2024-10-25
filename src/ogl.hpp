
#include <raylib.h>
#include <rlgl.h>

RenderTexture2D LoadRT32 (int w, int h) {
  RenderTexture2D target = { 0 };

  // Generate and bind framebuffer
  target.id = rlLoadFramebuffer(w, h);
  rlEnableFramebuffer(target.id);

  // Generate texture with 32-bit floating-point precision per channel
  target.texture.id = rlLoadTexture(NULL, w, h, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
  target.texture.width = w;
  target.texture.height = h;
  target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
  target.texture.mipmaps = 1;

  // Attach the 32-bit texture to the framebuffer
  rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

  // Generate and attach depth texture
  target.depth.id = rlLoadTextureDepth(w, h, true);
  rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

  // Verify framebuffer is complete
  if (rlFramebufferComplete(target.id)) {
      TraceLog(LOG_INFO, "Framebuffer created successfully");
  } else {
      TraceLog(LOG_WARNING, "Framebuffer creation failed");
  }

  // Unbind the framebuffer to go back to the default screen
  rlDisableFramebuffer();
}