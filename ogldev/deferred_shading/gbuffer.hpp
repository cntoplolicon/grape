#ifndef _G_BUFFER_H_
#define _G_BUFFER_H_ value

#include <stdexcept>
#include "GL/glus.h"

class GBuffer
{
    public:
        enum GBUFFER_TEXTURE_TYPE {
            GBUFFER_TEXTURE_TYPE_POSITION,
            GBUFFER_TEXTURE_TYPE_DIFFUSE,
            GBUFFER_TEXTURE_TYPE_NORMAL,
            GBUFFER_TEXTURE_TYPE_TEXCOORD,
            GBUFFER_NUM_TEXTURES
        };

        GBuffer(int windowWidth, int windowHeight)
        {
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

            glGenTextures(GBUFFER_NUM_TEXTURES, textures);
            glGenTextures(1, &depthTexture);

            GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3}; 
            for (int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
                glBindTexture(GL_TEXTURE_2D, textures[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, drawBuffers[i], GL_TEXTURE_2D, textures[i], 0);
            }

            glBindTexture(GL_TEXTURE_2D, depthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight, 
                    0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

            glDrawBuffers(GBUFFER_NUM_TEXTURES, drawBuffers);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                throw std::runtime_error("failed creating frame buffer");
            }

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }

        ~GBuffer()
        {
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(GBUFFER_NUM_TEXTURES, textures);
            glDeleteTextures(1, &depthTexture);
        }

        void bindForWriting()
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        }

        void bindForReading()
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        }

        void setReadBuffer(GBUFFER_TEXTURE_TYPE textureType)
        {
            glReadBuffer(GL_COLOR_ATTACHMENT0 + textureType);
        }

    private:
        GLuint fbo = 0;
        GLuint textures[GBUFFER_NUM_TEXTURES] = {0};
        GLuint depthTexture = 0;
};

#endif
