#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glus.h>
#include "rapidxml.hpp"

class ModelDataBuffer
{
    size_t _elementSize;
    size_t _count;
    size_t _size;
    void *_buffer;
public:
    size_t elementSize() { return _elementSize; }
    size_t count() { return _count; }
    size_t size() { return _size; }
    void * buffer() { return _buffer; }

    ModelDataBuffer(size_t _elementSize, size_t _count) : _elementSize(_elementSize), _count(_count), 
        _size(_elementSize * _count), _buffer(static_cast<void *>(new char[_size])) {}

    ~ModelDataBuffer()
    {
        if (_buffer) {
            delete[] ((char *)_buffer);
        }
    }

    ModelDataBuffer(const ModelDataBuffer &buf) = delete;
    ModelDataBuffer(ModelDataBuffer &&buf) : _elementSize(buf._elementSize), _count(buf._count),
        _size(buf._size), _buffer(buf._buffer)
    {
        buf._elementSize = buf._size = buf._count = 0;
        buf._buffer = nullptr;
    }
};

class AttributeType
{
public:
    AttributeType(const std::string &_typeName, bool _normalized, GLenum _glType) : 
        typeName(_typeName), normalized(_normalized), glType(_glType) {}

    const std::string typeName;
    const bool normalized;
    const GLenum glType;

    virtual ModelDataBuffer parse(const std::string &input) const = 0;
};

template <typename T>
class AttributeTypeImpl : public AttributeType
{
public:
    AttributeTypeImpl(const std::string &typeName, bool normalized, GLenum glType) : 
        AttributeType(typeName, normalized, glType) {}

    typedef T ValueType;
    virtual ModelDataBuffer parse(const std::string &input) const
    {
        std::stringstream inputStream(input, std::ios_base::in);
        inputStream.exceptions(std::stringstream::badbit);
        ValueType temp;
        std::vector<ValueType> values;
        while (inputStream >> temp) {
            values.push_back(temp);
        }

        ModelDataBuffer buffer(sizeof(ValueType), values.size());
        std::copy(values.begin(), values.end(), static_cast<ValueType *>(buffer.buffer()));
        return buffer;
    }
};

class Attribute
{
public:
    Attribute(int _index, int _size, const AttributeType *_type) :
        index(_index), size(_size), type(_type), offset(0) {}

    const GLuint index;
    const GLsizei size;
    const AttributeType *type;

    size_t offset;
};

class VertexArrayObject
{
    GLuint object;
    std::string _name;
    
public:
    std::string name() const
    {
        return _name;
    }

    VertexArrayObject(const VertexArrayObject &vao) = delete;

    VertexArrayObject(VertexArrayObject &&vao)
    {
        object = vao.object;
        vao.object = 0;
    }

    VertexArrayObject(const std::string &_name, const std::vector<Attribute> &attributes, 
            GLuint arrayBuffer, GLuint elementBuffer) : _name(_name)
    {
        GLint oldArrayBuffer, oldElementBuffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldArrayBuffer);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldElementBuffer);

        glGenVertexArrays(1, &object);
        glBindVertexArray(object);

        glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        for (const Attribute &attribute : attributes)
        {
            glEnableVertexAttribArray(attribute.index);
            glVertexAttribPointer(attribute.index, attribute.size, attribute.type->glType, 
                    attribute.type->normalized, 0, reinterpret_cast<void *>(attribute.offset));
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, oldArrayBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldElementBuffer);
    }

    ~VertexArrayObject()
    {
        if (object != 0) {
            glDeleteVertexArrays(1, &object);
        }
    }

    void bind() const
    {
        glBindVertexArray(object);
    }
};

class VertexArrayObjectBinder
{
    GLint bindingVertexArray;
public:
    VertexArrayObjectBinder(const VertexArrayObject *vao)
    {
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &bindingVertexArray);
        vao->bind();
    }

    ~VertexArrayObjectBinder()
    {
        glBindVertexArray(bindingVertexArray);
    }
};

class ArrayRenderer
{
public:
    const GLenum primitiveType;
    const GLint first;
    const GLsizei count;

    ArrayRenderer(GLenum _primitiveType, GLint _first, GLsizei _count) :
        primitiveType(_primitiveType), first(_first), count(_count) {}

    void render()
    {
        glDrawArrays(primitiveType, first, count);
    }
};

class IndexedRender
{
public:
    const GLenum primitiveType;
    const GLenum indexType; 
    const GLint count;
    size_t offset;

    IndexedRender(GLenum _primitiveType, GLenum _indexType, GLint _count) :
        primitiveType(_primitiveType), indexType(_indexType), count(_count) {}
    
    void setOffset(size_t _offset)
    {
        offset = _offset;
    }

    void render()
    {
        glDrawElements(primitiveType, count, indexType, reinterpret_cast<void *>(offset));
    }
};

class PrimitiveType
{
public:
    PrimitiveType(const std::string &_typeName, GLenum _glType) : typeName(_typeName), glType(_glType){}
    const std::string typeName;
    const GLenum glType;
};

class MeshTypeUtil
{
    const static AttributeTypeImpl<GLfloat> floatType;
    const static AttributeTypeImpl<GLint> intType;
    const static AttributeTypeImpl<GLuint> uintType;
    const static AttributeTypeImpl<GLint> normalizedIntType;
    const static AttributeTypeImpl<GLuint> normalizedUintType;
    const static AttributeTypeImpl<GLshort> shortType;
    const static AttributeTypeImpl<GLushort> ushortType;
    const static AttributeTypeImpl<GLshort> normalizedShortType;
    const static AttributeTypeImpl<GLushort> normalizedUshortType;
    const static AttributeTypeImpl<GLbyte> byteType;
    const static AttributeTypeImpl<GLubyte> ubyteType;
    const static AttributeTypeImpl<GLbyte> normalizedByteType;
    const static AttributeTypeImpl<GLubyte> normalizedUbyteType;

    const static AttributeType * attribTypes[];
    const static PrimitiveType primitiveTypes[];

public:
    const static AttributeType * getAttribType(const std::string &name);
    const static PrimitiveType * getPrimitiveType(const std:: string &name);
};

const AttributeTypeImpl<GLfloat> MeshTypeUtil::floatType("float", false, GL_FLOAT);
const AttributeTypeImpl<GLint> MeshTypeUtil::intType("int", false, GL_INT);
const AttributeTypeImpl<GLuint> MeshTypeUtil::uintType("uint", false, GL_UNSIGNED_INT);
const AttributeTypeImpl<GLint> MeshTypeUtil::normalizedIntType("norm-int", true, GL_INT);
const AttributeTypeImpl<GLuint> MeshTypeUtil::normalizedUintType("norm-uint", true, GL_UNSIGNED_INT);
const AttributeTypeImpl<GLshort> MeshTypeUtil::shortType("short", false, GL_SHORT);
const AttributeTypeImpl<GLushort> MeshTypeUtil::ushortType("ushort", false, GL_UNSIGNED_SHORT);
const AttributeTypeImpl<GLshort> MeshTypeUtil::normalizedShortType("norm-short", true, GL_SHORT);
const AttributeTypeImpl<GLushort> MeshTypeUtil::normalizedUshortType("norm-ushort", true, GL_UNSIGNED_SHORT);
const AttributeTypeImpl<GLbyte> MeshTypeUtil::byteType("byte", false, GL_BYTE);
const AttributeTypeImpl<GLubyte> MeshTypeUtil::ubyteType("ubyte", false, GL_UNSIGNED_BYTE);
const AttributeTypeImpl<GLbyte> MeshTypeUtil::normalizedByteType("norm-byte", true, GL_BYTE);
const AttributeTypeImpl<GLubyte> MeshTypeUtil::normalizedUbyteType("norm-ubyte", true, GL_UNSIGNED_BYTE);

const AttributeType *MeshTypeUtil::attribTypes[] = {&MeshTypeUtil::floatType, 
    &MeshTypeUtil::intType, &MeshTypeUtil::uintType, &MeshTypeUtil::normalizedIntType, &MeshTypeUtil::normalizedUintType,
    &MeshTypeUtil::shortType, &MeshTypeUtil::ushortType, &MeshTypeUtil::normalizedIntType, &MeshTypeUtil::normalizedUshortType,
    &MeshTypeUtil::byteType, &MeshTypeUtil::ubyteType, &MeshTypeUtil::normalizedByteType, &MeshTypeUtil::normalizedUbyteType
};

const AttributeType *MeshTypeUtil::getAttribType(const std::string &name)
{
    const AttributeType *ret = nullptr;
    std::for_each(attribTypes, attribTypes + sizeof(attribTypes) / sizeof(attribTypes[0]), 
            [&] (const AttributeType *attrib) { if (attrib->typeName == name) { ret = attrib; }});
    return ret;
}

const PrimitiveType MeshTypeUtil::primitiveTypes[] = {
    {"triangles", GL_TRIANGLES},
    {"tri-strip", GL_TRIANGLE_STRIP},
    {"tri-fan", GL_TRIANGLE_FAN},
    {"lines", GL_LINES},
    {"line-strip", GL_LINE_STRIP},
    {"line-loop", GL_LINE_LOOP},
    {"points", GL_POINTS}
};

const PrimitiveType * MeshTypeUtil::getPrimitiveType(const std::string &name)
{
    const PrimitiveType *ret = nullptr;
    std::for_each(primitiveTypes, primitiveTypes + sizeof(primitiveTypes) / sizeof(primitiveTypes[0]), 
            [&] (const PrimitiveType &type) { if (type.typeName == name) { ret = &type; }});
    return ret;
}

class Mesh
{
    std::vector<ModelDataBuffer> vertexData;
    std::vector<ModelDataBuffer> indexData;

    std::vector<Attribute> attributes;
    std::vector<IndexedRender> indexedRenderers;
    std::vector<ArrayRenderer> arrayRenderers;
    std::vector<VertexArrayObject> vertexArrayObjects;

    GLuint vertexBuffer;
    GLuint indexBuffer;

    void createAttribute(rapidxml::xml_node<> *node)
    {
        rapidxml::xml_attribute<> *typeNameAttr = node->first_attribute("type");
        if (!typeNameAttr) {
            throw std::runtime_error("no type specified for attribute");
        }
        std::string typeName = typeNameAttr->value();
        const AttributeType *type = MeshTypeUtil::getAttribType(typeName);
        if (!type) {
            throw std::runtime_error("unkown type of attribute: " + typeName);
        }

        rapidxml::xml_attribute<> *indexAttr = node->first_attribute("index");
        if (!indexAttr) {
            throw std::runtime_error("no index specified for attribute");
        }

        GLuint index = std::stoi(indexAttr->value());

        rapidxml::xml_attribute<> *sizeAttr = node->first_attribute("size");
        if (!sizeAttr) {
            throw std::runtime_error("no size specified for attribute");
        }
        GLsizei size = std::stoi(sizeAttr->value());

        attributes.push_back(Attribute(index, size, type));

        vertexData.push_back(type->parse(node->value()));
    }

    void createVertexBuffer()
    {
        size_t totalSize = 0;
        for (size_t i = 0; i < vertexData.size(); i++) {
            attributes[i].offset = totalSize;
            totalSize += vertexData[i].size();
        }

        GLint oldArrayBuffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldArrayBuffer);
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);
        for (size_t i = 0; i < vertexData.size(); i++) {
            glBufferSubData(GL_ARRAY_BUFFER, attributes[i].offset, vertexData[i].size(), vertexData[i].buffer());
        }
        glBindBuffer(GL_ARRAY_BUFFER, oldArrayBuffer);
    }

    void createIndexedRenderer(const rapidxml::xml_node<> *node)
    {
        rapidxml::xml_attribute<> *cmdAttr = node->first_attribute("cmd");
        if (!cmdAttr) {
            throw std::runtime_error("no primitive type specified for indices");
        }
        std::string primTypeName = cmdAttr->value();
        const PrimitiveType *primitiveType = MeshTypeUtil::getPrimitiveType(primTypeName);
        if (!primitiveType) {
            throw std::runtime_error("unkown pritimive type for indices: " + primTypeName);
        }

        rapidxml::xml_attribute<> *typeAttr = node->first_attribute("type");
        if (!typeAttr) {
            throw std::runtime_error("no data type specified or indices");
        }
        std::string dataTypeName = typeAttr->value();
        const AttributeType *dataType = MeshTypeUtil::getAttribType(dataTypeName);
        if (!dataType) {
            throw std::runtime_error("unkown data type for indices: " + dataTypeName);
        }

        indexData.push_back(dataType->parse(node->value()));
        indexedRenderers.push_back(IndexedRender(primitiveType->glType, dataType->glType, indexData[indexData.size() - 1].count()));
    }

    void createArrayRenderer(const rapidxml::xml_node<> *node)
    {
        rapidxml::xml_attribute<> *cmdAttr = node->first_attribute("cmd");
        if (!cmdAttr) {
            throw std::runtime_error("no primitive type specified for array");
        }
        std::string typeName = cmdAttr->value();
        const PrimitiveType *type = MeshTypeUtil::getPrimitiveType(typeName);
        if (!type) {
            throw std::runtime_error("unkown pritimive type for array: " + typeName);
        }

        rapidxml::xml_attribute<> *startAttr = node->first_attribute("start");
        if (!startAttr) {
            throw std::runtime_error("no start index specified for array");
        }
        GLuint start = std::stoi(startAttr->value());

        rapidxml::xml_attribute<> *countAttr = node->first_attribute("count");
        if (!countAttr) {
            throw std::runtime_error("no count specified for array");
        }
        GLsizei count = std::stoi(countAttr->value());

        arrayRenderers.push_back(ArrayRenderer(type->glType, start, count));
    }

    void createIndexBuffer()
    {
        if (indexData.empty()) {
            return;
        }

        size_t totalSize = 0;
        for (size_t i = 0; i < indexData.size(); i++) {
            indexedRenderers[i].offset = totalSize;
            totalSize += indexData[i].size();
        }

        GLint oldElementBuffer;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldElementBuffer);
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);
        for (size_t i = 0; i < vertexData.size(); i++) {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexedRenderers[i].offset, indexData[i].size(), indexData[i].buffer());
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldElementBuffer);
    }

    VertexArrayObject createVertexArrayObject(const rapidxml::xml_node<> *node)
    {
        rapidxml::xml_attribute<> *nameAttr = node->first_attribute("name");
        if (!nameAttr) {
            throw new std::runtime_error("no name specified for vao");
        }
        std::string name = nameAttr->value();

        std::vector<Attribute> attrsForVao;
        for (rapidxml::xml_node<> *sourceNode = node->first_node("source");
                sourceNode; sourceNode = sourceNode->next_sibling("source")) {
            rapidxml::xml_attribute<> *attribIndexAttr = sourceNode->first_attribute("attrib");
            if (!attribIndexAttr) {
                throw new std::runtime_error("no index specified for attribute source");
            }
            GLuint index = std::stoi(attribIndexAttr->value());
            std::for_each(attributes.begin(), attributes.end(), 
                    [&] (const Attribute &a) { if (a.index == index) { attrsForVao.push_back(a); } });
        }

        if (attrsForVao.empty()) {
            throw new std::runtime_error("no attribute source specified for vao");
        }
        return VertexArrayObject(name, attrsForVao, vertexBuffer, indexBuffer);
    }

    void createVertexArrayObjects(const rapidxml::xml_node<> *root)
    {
        std::vector<rapidxml::xml_node<> *> nodes;
        for (rapidxml::xml_node<> *node = root->first_node("vao");
                node != nullptr; node = node->next_sibling("vao")) {
            nodes.push_back(node);
        }

        vertexArrayObjects.push_back(VertexArrayObject("", attributes, vertexBuffer, indexBuffer));
        for (size_t i = 0; i < nodes.size(); i++) {
            vertexArrayObjects.push_back(createVertexArrayObject(nodes[i]));
        }
    }

public:
    Mesh(const std::string &meshFile)
    {
        std::ifstream file(meshFile);
        std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        rapidxml::xml_document<> document;
        document.parse<0>(const_cast<char *>(text.c_str()));
        rapidxml::xml_node<> *root = document.first_node();
        if (!root) {
            throw std::runtime_error("no mesh data found");
        }

        for (rapidxml::xml_node<> *node = root->first_node("attribute"); 
                node != nullptr; node = node->next_sibling("attribute")) {
            createAttribute(node);
        }
        createVertexBuffer();

        for (rapidxml::xml_node<> *node = root->first_node("indices");
                node != nullptr; node = node->next_sibling("indices")) {
            createIndexedRenderer(node);
        }
        createIndexBuffer();

        for (rapidxml::xml_node<> *node = root->first_node("arrays"); 
                node != nullptr; node = node->next_sibling("arrays")) {
            createArrayRenderer(node);    
        }

        createVertexArrayObjects(root);
    }

    ~Mesh()
    {
        glDeleteBuffers(1, &indexBuffer);
        glDeleteBuffers(1, &vertexBuffer);
    }

    void render()
    {
        VertexArrayObjectBinder binder(&vertexArrayObjects[0]);
        std::for_each(indexedRenderers.begin(), indexedRenderers.end(), std::mem_fn(&IndexedRender::render));
        std::for_each(arrayRenderers.begin(), arrayRenderers.end(), std::mem_fn(&ArrayRenderer::render));
    }

    void render(const std::string &name)
    {
        const VertexArrayObject *v = nullptr;
        std::for_each(vertexArrayObjects.begin(), vertexArrayObjects.end(), 
                [&] (const VertexArrayObject &vao) { if (vao.name() == name) { v = &vao; }});
        if (v == nullptr) {
            throw std::runtime_error("unknown vao name: " + name);
        }
        VertexArrayObjectBinder binder(v);
        std::for_each(indexedRenderers.begin(), indexedRenderers.end(), std::mem_fn(&IndexedRender::render));
        std::for_each(arrayRenderers.begin(), arrayRenderers.end(), std::mem_fn(&ArrayRenderer::render));
    }
};

