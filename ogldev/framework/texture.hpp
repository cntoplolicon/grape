#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <iostream>
#include <GL/glus.h>
#include <string>
#include <ImageMagick-6/Magick++.h>

class Texture
{
    GLenum m_textureTarget;
    GLuint m_textureObj;
public:
    Texture(GLenum textureTarget, const std::string &filename)
    {
        Magick::Blob m_blob;
        Magick::Image m_image;
        try {
            m_image.read(filename);
            m_image.write(&m_blob, "RGBA");
        }
        catch (Magick::Error& Error) {
            std::cerr << "Error loading texture '" << filename << "': " << Error.what() << std::endl;
            throw Error;
        }

        m_textureTarget = textureTarget;
        glGenTextures(1, &m_textureObj);
        glBindTexture(m_textureTarget, m_textureObj);
        glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.columns(), m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
        glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
        glBindTexture(m_textureTarget, 0);
    }

    ~Texture()
    {
        glBindTexture(m_textureTarget, 0);
        glDeleteTextures(1, &m_textureObj);
    }

    void Bind(GLenum textureUnit)
    {
        glActiveTexture(textureUnit);
        glBindTexture(m_textureTarget, m_textureObj);
    }
};

#endif
