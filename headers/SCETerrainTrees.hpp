/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCETerrainTrees.hpp*******/
/**************************************/
#ifndef SCE_SCETERRAINTREES_HPP
#define SCE_SCETERRAINTREES_HPP

#include "SCEDefines.hpp"
#include <vector>

#define TREE_LOD_COUNT 3
#define TREE_LOD_MIN 0

namespace SCE
{
    class TerrainTrees
    {
    public :

        TerrainTrees();
        ~TerrainTrees();
        void InitializeTreeLayout(glm::vec4 *normAndHeightTex, int textureSize, float xOffset, float zOffset,
                                  float startScale, float heightScale, float halfTerrainSize);
        void SpawnTreeInstances(const glm::mat4 &viewMatrix, const glm::mat4 &worldToTerrainspaceMatrix,
                                const glm::vec3 &cameraPosition, float maxDistFromCenter);
        void RenderTrees(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, bool isShadowPass);

    private :

        struct ImpostorGLData
        {
            ui16        meshId;
            GLuint      texture;
            GLint       textureUniform;
            GLint       scaleInvertMatUniform;
            GLuint      normalTexture;
            GLint       normalUniform;
            GLuint      shaderProgram;
            glm::mat4   scaleMatrix;
        };

        struct TreeGLData
        {            
            GLuint  trunkShaderProgram;
            GLuint  leavesShaderProgram;

            GLuint  leafTexture;
            GLuint  barkTexture;
            GLuint  barkNormalTexture;

            GLuint  leafTexUniform;
            GLuint  barkTexUniform;
            GLuint  barkNormalTexUniform;

            ui16    trunkMeshIds[TREE_LOD_COUNT];
            ui16    leavesMeshIds[TREE_LOD_COUNT];

            ImpostorGLData impostorData;
        };

        struct TreeGroup
        {
            glm::vec2 position;
            float radius;
            float spacing;
        };

        TreeGLData              treeGlData;
        std::vector<TreeGroup>  treeGroups;

    };
}

#endif
