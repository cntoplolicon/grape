#ifndef __UNIFORM_BLOCK_ARRAY__
#define __UNIFORM_BLOCK_ARRAY__

#include <GL/glus.h>

template<typename value_type, int COUNT>
class UniformBlockArray
{
    int m_offset;
    char *buffer;
public:
    UniformBlockArray() : m_offset(0), buffer(nullptr)
    {
        GLint alignment = 0;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

        m_offset = sizeof(value_type);
        m_offset += alignment - (m_offset % alignment);

        buffer = reinterpret_cast<char *>(malloc(m_offset * COUNT));
        memset(buffer, 0, m_offset * COUNT);
    }

    ~UniformBlockArray()
    {
        free(buffer);
    } 

    value_type & operator[] (const int index)
    {
        value_type *pt = reinterpret_cast<value_type *>(buffer + m_offset * index);
        return *pt;
    }

    int offset() const
    {
        return m_offset;
    }

    int count() const
    {
        return count;
    }

    GLuint createBuffer()
    {
        GLuint bufferObject;
        glGenBuffers(1, &bufferObject);
        glBindBuffer(GL_UNIFORM_BUFFER, bufferObject);
        glBufferData(GL_UNIFORM_BUFFER, m_offset * COUNT, buffer, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        return bufferObject;
    }
};

#endif

