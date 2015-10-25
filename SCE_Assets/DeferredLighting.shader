[VertexShader]
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec3 vertexNormal_modelspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);
    }
_}

[FragmentShader]
_{
#version 430 core

    #define CASCADE_COUNT 4
    #define SHADOW_MAP_SIZE 2048.0

    uniform vec2    SCE_ScreenSize;
    uniform vec3    SCE_EyePosition_worldspace;
    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_LightMaxDotAngle;
    uniform float   SCE_ShadowStrength;

    uniform mat4        DepthConvertMat[CASCADE_COUNT];
    uniform mat4        V;
    uniform mat4        P;

    uniform float       FarSplits_cameraspace[CASCADE_COUNT];

    uniform sampler2D   PositionTex;
    uniform sampler2D   DiffuseTex;
    uniform sampler2D   NormalTex;
    //    uniform sampler2D   ShadowTex;
    uniform sampler2DArrayShadow ShadowTex;

    out vec4 color;


#define LIGHT_SUBROUTINE_PARAMS \
    in vec3 in_Position_worldspace,\
    in vec3 in_LightDirection_worldspace,\
    in vec3 in_Normal_worldspace,\
    in vec3 in_EyeToFrag_worldspace,\
    in vec3 in_LightToFrag_worldspace,\
    in float in_LightReach_worldspace,\
    in float in_Light_Specular\


    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = max(fromMin, (min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return mix(toMin, toMax, val);
    }

    float getShadowDepth(vec3 pos_worldspace, vec3 normal_worldspace, vec3 lightDir_worldspace);

    //define a subroutine signature
    subroutine vec3 SCE_ComputeLightType(LIGHT_SUBROUTINE_PARAMS);

    //Directional light option
    subroutine (SCE_ComputeLightType) vec3 SCE_ComputeDirectionalLight(LIGHT_SUBROUTINE_PARAMS)
    {
        //Diffuse component
        vec3 dirToLight = normalize(-in_LightDirection_worldspace);
        float NdotL     = dot(in_Normal_worldspace, dirToLight);
        NdotL           = clamp(NdotL, 0, 1);

        vec3 dirToEye       = normalize(-in_EyeToFrag_worldspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_worldspace, halway), 0.0 ,1.0 );

        float shadow = getShadowDepth(in_Position_worldspace,
                                      in_Normal_worldspace,
                                      SCE_LightDirection_worldspace);
        shadow *= SCE_ShadowStrength;

        vec3 light  = vec3(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0) * in_Light_Specular, //specular component
                    shadow);

        return light;
    }

    //Point light option
    subroutine (SCE_ComputeLightType) vec3 SCE_ComputePointLight(LIGHT_SUBROUTINE_PARAMS)
    {
        //Diffuse component
        vec3 dirToLight = normalize(-in_LightToFrag_worldspace);
        float NdotL     = dot(in_Normal_worldspace, dirToLight);
        NdotL           = clamp(NdotL, 0.0, 1.0);

        vec3 dirToEye       = normalize(-in_EyeToFrag_worldspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_worldspace, halway), 0.0 ,1.0 );

        float lightReach    = in_LightReach_worldspace;

        //hackish attenuation, but works well
        float dist          = length(in_LightToFrag_worldspace);
        float d             = mapToRange(0.0, lightReach, 1.0, 0.0, dist);
        float attenuation   = d*d;

        vec3 light  = vec3(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0) * in_Light_Specular, //specular component
                    0.0);

        light *= attenuation;
        return light;
    }

    //Spot light option
    subroutine (SCE_ComputeLightType) vec3 SCE_ComputeSpotLight(LIGHT_SUBROUTINE_PARAMS)
    {
        //Diffuse component
        vec3 dirToLight     = normalize(-in_LightToFrag_worldspace);
        vec3 invLightDir    = normalize(-in_LightDirection_worldspace);
        float NdotL         = dot(in_Normal_worldspace, dirToLight);
        NdotL               = clamp(NdotL, 0.0, 1.0);

        vec3 dirToEye       = normalize(-in_EyeToFrag_worldspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_worldspace, halway), 0.0 ,1.0 );

        float lightReach    = in_LightReach_worldspace;

        //use very simple fallof approximation to fade spot light with distance
        float dist          = length(in_LightToFrag_worldspace);
        float d             = mapToRange(0.0, lightReach, 1.0, 0.0, dist);
        float attenuation   = d*d;

        float fragDotL = dot(dirToLight, invLightDir);
        float surfDotL = SCE_LightMaxDotAngle;

        float spotAttenuation = mapToRange(surfDotL, 1.0, 0.0, 1.0, fragDotL);
        spotAttenuation = pow(spotAttenuation, 4.0);

        vec3 light  = vec3(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0) * in_Light_Specular, //specular component
                    0.0);

        light *= attenuation * spotAttenuation;
        return light;
    }

    //uniform variable declaration for the light function subroutine
    subroutine uniform SCE_ComputeLightType SCE_ComputeLight;


    vec2 poissonDisk[16] = vec2[](
            vec2( -0.94201624, -0.39906216 ),
            vec2( 0.94558609, -0.76890725 ),
            vec2( -0.094184101, -0.92938870 ),
            vec2( 0.34495938, 0.29387760 ),
            vec2( -0.91588581, 0.45771432 ),
            vec2( -0.81544232, -0.87912464 ),
            vec2( -0.38277543, 0.27676845 ),
            vec2( 0.97484398, 0.75648379 ),
            vec2( 0.44323325, -0.97511554 ),
            vec2( 0.53742981, -0.47373420 ),
            vec2( -0.26496911, -0.41893023 ),
            vec2( 0.79197514, 0.19090188 ),
            vec2( -0.24188840, 0.99706507 ),
            vec2( -0.81409955, 0.91437590 ),
            vec2( 0.19984126, 0.78641367 ),
            vec2( 0.14383161, -0.14100790 )
         );

    //rand [0, 1[
    float rand(vec4 seed4)
    {
        float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
        return fract(sin(dot_product) * 43758.5453);
    }

    float sampleMyTex(vec2 uv, float cascade, float z)
    {
        return texture(ShadowTex, vec4(uv, cascade, z));
    }

    float samplePoisson(int sampleCount, float z_depthspace, vec2 uv,
                        vec3 pos_worldspace, float cascade)
    {
        int poissonSize = 16;
        float poissonSpread = 1.0 / 1200.0;
        float shadow = 0.0;
        vec4 seed4 = vec4(0.0);
        float fSamples = float(sampleCount);
        int index = 0;
        float cascadeNum = cascade + 1.0;
        for (int off = 0; off < sampleCount; off++)
        {
            seed4 = vec4(floor(pos_worldspace * 500.0 / cascadeNum), off);
            index = int((float(poissonSize)*rand(seed4))) % poissonSize;
            uv += poissonDisk[index] * poissonSpread / cascadeNum;
            shadow += (1.0 - sampleMyTex(uv, cascade, z_depthspace)) / fSamples;
        }

        return shadow;
    }

    float samplePCF(int sampleCount, float z_depthspace, vec2 uv, float cascade)
    {
        float fsamples = float(sampleCount);
        float kernelSize = sqrt(fsamples);
        float shadow = 0.0;

        for(float off = 0.0; off < fsamples; ++off)
        {
            float u = floor(off / kernelSize);
            float v = mod(off, kernelSize);
            vec2 uvOffset = vec2(u, v) / SHADOW_MAP_SIZE;
            shadow += (1.0 - sampleMyTex(uv + uvOffset, cascade, z_depthspace)) / fsamples;
        }

        return shadow;
    }

    #define SHADOW_BIAS 0.0005

    float getShadowDepth(vec3 pos_worldspace, vec3 normal_worldspace, vec3 lightDir_worldspace)
    {
        int sampleCount = 4;
        float eyeDist = distance(pos_worldspace, SCE_EyePosition_worldspace);
        vec3 pos_cameraspace = (V * vec4(pos_worldspace, 1.0)).xyz;

        float biasOffset = min(1.0 - dot(-lightDir_worldspace, normal_worldspace), 0.5);
        float bias = 0.0;//SHADOW_BIAS * biasOffset;

        float shadow = 0.0;
        vec4 position_depthspace = vec4(0.0);

        for(int i = 0; i < CASCADE_COUNT; ++i)
        {
            position_depthspace = DepthConvertMat[i] * vec4(pos_worldspace, 1.0);

            if(pos_cameraspace.z <= FarSplits_cameraspace[i])
            {
    //              shadow = samplePoisson(sampleCount, position_depthspace.z, position_depthspace.xy,
    //                                      pos_worldspace, float(i));
                shadow = samplePCF(sampleCount, position_depthspace.z - bias,
                                   position_depthspace.xy, float(i));
                return shadow;
            }
        }
        return shadow;
    }

//#define DEBUG

    void main()
    {
        vec3 ambiantColor = vec3(1.0, 1.0, 1.0) * 0.005;

        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 Position_worldspace    = texture2D(PositionTex, uv).xyz;

        //bail early if texture wasn't filled at this position
        if(dot(Position_worldspace, Position_worldspace) < 0.0001)
        {
            color = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        vec3 MaterialDiffuseColor   = texture2D(DiffuseTex, uv).xyz;
        vec4 normSpec               = texture2D(NormalTex, uv);
        vec3 Normal_worldspace      = normSpec.xyz;

        float Specularity           = normSpec.a;

        color = vec4(MaterialDiffuseColor, 1.0);

        //Position worldspace (0,0,0), no object rendered here, use color directly;

        vec3 LightToFrag_cameraspace = Position_worldspace - SCE_LightPosition_worldspace;
        vec3 EyeToFrag_cameraspace = Position_worldspace - SCE_EyePosition_worldspace;

        vec3 lightCol = SCE_ComputeLight(
                    Position_worldspace,
                    SCE_LightDirection_worldspace,
                    Normal_worldspace,
                    EyeToFrag_cameraspace,
                    LightToFrag_cameraspace,
                    SCE_LightReach_worldspace,
                    Specularity
                    );


        color.rgb =
                //Diffuse
                (MaterialDiffuseColor * lightCol.x * SCE_LightColor.rgb * SCE_LightColor.a)
                //Specular
                + (SCE_LightColor.rgb * lightCol.y * lightCol.x * SCE_LightColor.a);
        //shadow and ambiant
        color.rgb = color.rgb * (1.0 - lightCol.z)
                     + SCE_ShadowStrength * ambiantColor * MaterialDiffuseColor;

#ifdef DEBUG
//        color = vec4(MaterialDiffuseColor, 1.0);
        color = vec4(Normal_worldspace, 1.0);
#endif
    }
_}
