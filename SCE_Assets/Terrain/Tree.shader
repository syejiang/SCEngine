[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;

    out vec2 fragUV;
    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;
    out vec3 Position_modelspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;
    uniform float PatchSize;
    uniform mat4 TreeToTerrainSpace;
    uniform sampler2D TerrainHeightMap;

    void main()
    {
        Position_modelspace = vertexPosition_modelspace;

        vec4 pos_terrainspace = TreeToTerrainSpace * vec4(0.0, 0.0, 0.0, 1.0);
        float height = texture(TerrainHeightMap, pos_terrainspace.zx * 0.5 + vec2(0.5)).a;

        vec3 movedVertexPos = vertexPosition_modelspace;
        movedVertexPos.y += (height - 0.005) * PatchSize;

        fragUV = vertexUV;
        Position_worldspace = ( M * vec4(movedVertexPos, 1.0) ).xyz;
        Normal_worldspace = ( M * vec4(vertexNormal_modelspace, 0.0) ).xyz;

        gl_Position = MVP * vec4(movedVertexPos, 1.0);
    }
_}

[FragmentShader]
_{
#version 400 core

    in vec2 fragUV;
    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;
    in vec3 Position_modelspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

//    uniform vec3  LeavesColor;
//    uniform vec3  TruncColor;
//    uniform float Specularity;

    vec3 LeavesColor = vec3(0.0, 1.0, 0.0);
    vec3 TruncColor = vec3(0.8, 0.3, 0.0);
    float Specularity = 1.0;

    void main()
    {
        float trunc = step(Position_modelspace.y, 3.0);
        oColor = trunc * TruncColor + (1.0 - trunc) * LeavesColor;

        oPosition = Position_worldspace;

        oNormal.xyz = Normal_worldspace;
        oNormal.a = Specularity;
    }
_}