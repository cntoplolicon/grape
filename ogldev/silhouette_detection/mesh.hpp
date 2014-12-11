/*

	Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MESH_H
#define	MESH_H

#include <map>
#include <vector>
#include <assert.h>
#include <Importer.hpp>      // C++ importer interface
#include <scene.h>       // Output data structure
#include <postprocess.h> // Post processing flags

#include "math.hpp"
#include "texture.hpp"

using namespace std;

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

struct Vector2f
{
    float x, y;
};

// Stores an edge by its vertices and force an order between them
struct Edge
{
    Edge(uint _a, uint _b)
    {
        assert(_a != _b);
        
        if (_a < _b)
        {
            a = _a;
            b = _b;                   
        }
        else
        {
            a = _b;
            b = _a;
        }
    }

    void Print()
    {
        printf("Edge %d %d\n", a, b);
    }
    
    uint a;
    uint b;
};

struct Neighbors
{
    uint n1;
    uint n2;
    
    Neighbors()
    {
        n1 = n2 = (uint)-1;
    }
    
    void AddNeigbor(uint n)
    {
        if (n1 == -1) {
            n1 = n;
        }
        else if (n2 == -1) {
            n2 = n;
        }
        else {
            assert(0);
        }
    }
    
    uint GetOther(uint me) const
    {
        if (n1 == me) {
            return n2;
        }
        else if (n2 == me) {
            return n1;
        }
        else {
            assert(0);
        }

        return 0;
    }
};

struct CompareEdges
{
    bool operator()(const Edge& Edge1, const Edge& Edge2) const
    {
        if (Edge1.a < Edge2.a) {
            return true;
        }
        else if (Edge1.a == Edge2.a) {
            return (Edge1.b < Edge2.b);
        }        
        else {
            return false;
        }            
    }
};


struct CompareVectors
{
    bool operator()(const aiVector3D& a, const aiVector3D& b) const
    {
        if (a.x < b.x) {
            return true;
        }
        else if (a.x == b.x) {
            if (a.y < b.y) {
                return true;
            }
            else if (a.y == b.y) {
                if (a.z < b.z) {
                    return true;
                }
            }
        }
        
        return false;
    }
};


struct Face
{
    uint Indices[3];
    
    uint GetOppositeIndex(const Edge& e) const
    {
        for (uint i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(Indices) ; i++) {
            uint Index = Indices[i];
            
            if (Index != e.a && Index != e.b) {
                return Index;
            }
        }
        
        assert(0);

        return 0;
    }
};


class Mesh
{
public:
    Mesh();

    ~Mesh();

    bool LoadMesh(const string& Filename, bool WithAdjacencies);

    void Render();
	
private:
    bool InitFromScene(const aiScene* pScene, const string& Filename);
    void FindAdjacencies(const aiMesh* paiMesh, vector<unsigned int>& Indices);
    void InitMesh(uint MeshIndex,
                  const aiMesh* paiMesh,
                  vector<Vector3f>& Positions,
                  vector<Vector3f>& Normals,
                  vector<Vector2f>& TexCoords,
                  vector<unsigned int>& Indices);
    bool InitMaterials(const aiScene* pScene, const string& Filename);
    void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF
  
enum VB_TYPES {
    INDEX_BUFFER,
    POS_VB,
    NORMAL_VB,
    TEXCOORD_VB,
    BONE_VB,
    NUM_VBs            
};

    GLuint m_VAO;
    GLuint m_Buffers[NUM_VBs];

    struct MeshEntry {
        MeshEntry()
        {
            NumIndices    = 0;
            BaseVertex    = 0;
            BaseIndex     = 0;
            MaterialIndex = INVALID_MATERIAL;
        }
        
        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };
    
    vector<MeshEntry> m_Entries;
    vector<Texture*> m_Textures;
     
    std::map<Edge, Neighbors, CompareEdges> m_indexMap;
    std::map<aiVector3D, uint, CompareVectors> m_posMap;    
    std::vector<Face> m_uniqueFaces;
    bool m_withAdjacencies;

    const aiScene* m_pScene;
    Assimp::Importer m_Importer;
};

#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   1
#define NORMAL_LOCATION      2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4


Mesh::Mesh()
{
    m_VAO = 0;
    memset(m_Buffers, 0, sizeof(m_Buffers));
    m_pScene = NULL;
    m_withAdjacencies = false;
}


Mesh::~Mesh()
{
    Clear();
}


void Mesh::Clear()
{
    for (uint i = 0 ; i < m_Textures.size() ; i++) {
        delete m_Textures[i];
    }

    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }
       
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}


bool Mesh::LoadMesh(const string& Filename, bool WithAdjacencies)
{
    m_withAdjacencies = WithAdjacencies;
    
    // Release the previously loaded mesh (if it exists)
    Clear();
 
    // Create the VAO
    glGenVertexArrays(1, &m_VAO);   
    glBindVertexArray(m_VAO);
    
    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    bool Ret = false;    
  
    m_pScene = m_Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | 
                                                     aiProcess_GenSmoothNormals | 
                                                     aiProcess_FlipUVs |
                                                     aiProcess_JoinIdenticalVertices);    
    if (m_pScene) {  
        Ret = InitFromScene(m_pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);	

    return Ret;
}


bool Mesh::InitFromScene(const aiScene* pScene, const string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    vector<Vector3f> Positions;
    vector<Vector3f> Normals;
    vector<Vector2f> TexCoords;
    vector<uint> Indices;
       
    uint NumVertices = 0;
    uint NumIndices = 0;
    
    uint VerticesPerPrim = m_withAdjacencies ? 6 : 3;
    
    // Count the number of vertices and indices
    for (uint i = 0 ; i < m_Entries.size() ; i++) {
        m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;        
        m_Entries[i].NumIndices    = pScene->mMeshes[i]->mNumFaces * VerticesPerPrim;
        m_Entries[i].BaseVertex    = NumVertices;
        m_Entries[i].BaseIndex     = NumIndices;
        
        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Entries[i].NumIndices;
    }
       
    // Reserve space in the vectors for the vertex attributes and indices
    Positions.reserve(NumVertices);
    Normals.reserve(NumVertices);
    TexCoords.reserve(NumVertices);
    Indices.reserve(NumIndices);
        
    // Initialize the meshes in the scene one by one
    for (uint i = 0 ; i < m_Entries.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh, Positions, Normals, TexCoords, Indices);
    }

    if (!InitMaterials(pScene, Filename)) {
        return false;
    }

    // Generate and populate the buffers with vertex attributes and the indices
  	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);    

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

   	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

    return glGetError() == GL_NO_ERROR;
}

void Mesh::FindAdjacencies(const aiMesh* paiMesh, vector<unsigned int>& Indices)
{       
    // Step 1 - find the two triangles that share every edge
    for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& face = paiMesh->mFaces[i];

        Face Unique;
        
        // If a position vector is duplicated in the VB we fetch the 
        // index of the first occurrence.
        for (uint j = 0 ; j < 3 ; j++) {            
            uint Index = face.mIndices[j];
            aiVector3D& v = paiMesh->mVertices[Index];
            
            if (m_posMap.find(v) == m_posMap.end()) {
                m_posMap[v] = Index;
            }
            else {
                Index = m_posMap[v];
            }           
            
            Unique.Indices[j] = Index;
        }
        
        m_uniqueFaces.push_back(Unique);
        
        Edge e1(Unique.Indices[0], Unique.Indices[1]);
        Edge e2(Unique.Indices[1], Unique.Indices[2]);
        Edge e3(Unique.Indices[2], Unique.Indices[0]);
        
        m_indexMap[e1].AddNeigbor(i);
        m_indexMap[e2].AddNeigbor(i);
        m_indexMap[e3].AddNeigbor(i);
    }   

    // Step 2 - build the index buffer with the adjacency info
    for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {        
        const Face& face = m_uniqueFaces[i];
        
        for (uint j = 0 ; j < 3 ; j++) {            
            Edge e(face.Indices[j], face.Indices[(j + 1) % 3]);
            assert(m_indexMap.find(e) != m_indexMap.end());
            Neighbors n = m_indexMap[e];
            uint OtherTri = n.GetOther(i);
            
            assert(OtherTri != -1);

            const Face& OtherFace = m_uniqueFaces[OtherTri];
            uint OppositeIndex = OtherFace.GetOppositeIndex(e);
         
            Indices.push_back(face.Indices[j]);
            Indices.push_back(OppositeIndex);             
        }
    }    
}


void Mesh::InitMesh(uint MeshIndex,
                    const aiMesh* paiMesh,
                    vector<Vector3f>& Positions,
                    vector<Vector3f>& Normals,
                    vector<Vector2f>& TexCoords,
                    vector<uint>& Indices)
{    
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    
    // Populate the vertex attribute vectors
    for (uint i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        Positions.push_back(Vector3f{pPos->x, pPos->y, pPos->z});
        Normals.push_back(Vector3f{pNormal->x, pNormal->y, pNormal->z});
        TexCoords.push_back(Vector2f{pTexCoord->x, pTexCoord->y});        
    }
    
	if (m_withAdjacencies) {
    	FindAdjacencies(paiMesh, Indices);
	}
	else {
	    // Populate the index buffer
	    for (uint i = 0 ; i < paiMesh->mNumFaces ; i++) {
	        const aiFace& Face = paiMesh->mFaces[i];
	        assert(Face.mNumIndices == 3);
	        Indices.push_back(Face.mIndices[0]);
	        Indices.push_back(Face.mIndices[1]);
	        Indices.push_back(Face.mIndices[2]);
	    }	
	}
}


bool Mesh::InitMaterials(const aiScene* pScene, const string& Filename)
{
    // Extract the directory part from the file name
    string::size_type SlashIndex = Filename.find_last_of("/");
    string Dir;

    if (SlashIndex == string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

    // Initialize the materials
    for (uint i = 0 ; i < pScene->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                string p(Path.data);
                
                if (p.substr(0, 2) == ".\\") {                    
                    p = p.substr(2, p.size() - 2);
                }
                               
                string FullPath = Dir + "/" + p;
                    
                m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());
            }
        }
    }

    return Ret;
}


void Mesh::Render()
{
    glBindVertexArray(m_VAO);
    
    uint Topology = m_withAdjacencies ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;
    
    for (uint i = 0 ; i < m_Entries.size() ; i++) {
        const uint MaterialIndex = m_Entries[i].MaterialIndex;

        assert(MaterialIndex < m_Textures.size());
        
        if (m_Textures[MaterialIndex]) {
            m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
        }                
        
		glDrawElementsBaseVertex(Topology, 
                                 m_Entries[i].NumIndices, 
                                 GL_UNSIGNED_INT, 
                                 (void*)(sizeof(uint) * m_Entries[i].BaseIndex), 
                                 m_Entries[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside    
    glBindVertexArray(0);
}

#endif	/* MESH_H */

