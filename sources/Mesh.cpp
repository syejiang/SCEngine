/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/*********** FILE : Mesh.cpp **********/
/**************************************/

#include "../headers/Mesh.hpp"
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/tangentspace.hpp>

using namespace std;


SCE::Mesh::Mesh(std::vector<ushort> *indices
                , std::vector<vec3> *vertices
                , std::vector<vec3> *normals
                , std::vector<vec2> *uvs
                , std::vector<vec3> *tangents
                , std::vector<vec3> *bitangents)
{
    resetMeshData();

    if(indices)
        mIndices    = new vector<ushort>(*indices);
    if(vertices)
        mVertices   = new vector<vec3>(*vertices);
    if(normals)
        mNormals    = new vector<vec3>(*normals);
    if(uvs)
        mUvs        = new vector<vec2>(*uvs);
    if(tangents)
        mTangents   = new vector<vec3>(*tangents);
    if(bitangents)
        mBitangents = new vector<vec3>(*bitangents);
}

SCE::Mesh::~Mesh()
{
    SECURE_DELETE(mIndices);
    SECURE_DELETE(mVertices);
    SECURE_DELETE(mUvs);
    SECURE_DELETE(mNormals);
    SECURE_DELETE(mTangents);
    SECURE_DELETE(mBitangents);
}

std::vector<ushort> *SCE::Mesh::GetIndices()
{
    return mIndices;
}

std::vector<vec3> *SCE::Mesh::GetVertices()
{
    return mVertices;
}

std::vector<vec3> *SCE::Mesh::GetNormals()
{
    return mNormals;
}

std::vector<vec2> *SCE::Mesh::GetUvs()
{
    return mUvs;
}

std::vector<vec3> *SCE::Mesh::GetTangents()
{
    return mTangents;
}

std::vector<vec3> *SCE::Mesh::GetBitangents()
{
    return mBitangents;
}


void SCE::Mesh::resetMeshData()
{
    mIndices    = 0l;
    mVertices   = 0l;
    mNormals    = 0l;
    mUvs        = 0l;
    mTangents   = 0l;
    mBitangents = 0l;
}

SCE::Mesh *SCE::Mesh::LoadMesh(const string &filename)
{
    string fullPath = RESSOURCE_PATH + filename;
    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
//    vector<vec3> tangents;
//    vector<vec3> bitangents;

    Mesh* mesh = 0l;

    if(loadOBJ(fullPath.c_str()
               , vertices
               , uvs
               , normals))
    {
        //computeTangentBasis(vertices, uvs, normals, tangents, bitangents);

        vector<ushort> indices;
        vector<vec3>   out_vert;
        vector<vec2>   out_uvs;
        vector<vec3>   out_norm;
//        vector<vec3>   out_tan;
//        vector<vec3>   out_bitan;

        indexVBO(vertices
                 , uvs
                 , normals
                 , indices
                 , out_vert
                 , out_uvs
                 , out_norm);

        mesh = new Mesh(&indices, &out_vert, &out_norm, &out_uvs, 0l, 0l);
    }

    return mesh;
}

SCE::Mesh *SCE::Mesh::CreateSphere(const ushort &radius, const ushort &tesselation)
{
    return 0l;
}

SCE::Mesh *SCE::Mesh::CreateBox(const ushort &width, const ushort &height, const ushort &depth)
{
    return 0l;
}

