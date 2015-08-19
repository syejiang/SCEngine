VertexShader : 
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec3 vertexNormal_modelspace;

    out vec3 LightDirection_cameraspace;
    out vec3 LightPosition_cameraspace;
    out float LightReach_cameraspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_SpotAttenuation;
    uniform float   SCE_LightCutoff;

    void main(){

        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);

        vec3 Position_worldspace    = (M * vec4(vertexPosition_modelspace, 1.0)).xyz;

        vec3 VertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace, 1.0)).xyz;

        LightPosition_cameraspace   = ( V * vec4(SCE_LightPosition_worldspace, 1.0)).xyz;

        LightDirection_cameraspace  = ( V * vec4(SCE_LightDirection_worldspace, 0.0)).xyz;

        LightReach_cameraspace = length( V * vec4(SCE_LightReach_worldspace, 0.0, 0.0, 0.0));
    }
_}

FragmentShader : 
_{
#version 430 core

    uniform vec2    SCE_ScreenSize;
    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_SpotAttenuation;
    uniform float   SCE_LightCutoff;

#define LIGHT_SUBROUTINE_PARAMS in vec3 in_LightDirection_cameraspace,\
    in vec3 in_Normal_cameraspace,\
    in vec3 in_EyeToFrag_cameraspace,\
    in vec3 in_LightToFrag_cameraspace,\
    in float in_LightReach_cameraspace


    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = max(fromMin, (min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return mix(toMin, toMax, val);
    }

    //define a subroutine signature
    subroutine vec2 SCE_ComputeLightType(LIGHT_SUBROUTINE_PARAMS);

    //Directional light option
    subroutine (SCE_ComputeLightType) vec2 SCE_ComputeDirectionalLight(LIGHT_SUBROUTINE_PARAMS) {

        //Diffuse component
        vec3 dirToLight = normalize(-in_LightDirection_cameraspace);
        float NdotL     = dot(in_Normal_cameraspace, dirToLight);
        NdotL           = clamp(NdotL, 0, 1);

        vec3 dirToEye       = normalize(-in_EyeToFrag_cameraspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_cameraspace, halway), 0.0 ,1.0 );

        vec2 light  = vec2(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0)); //specular component

        return light;
    }

    //Point light option
    subroutine (SCE_ComputeLightType) vec2 SCE_ComputePointLight(LIGHT_SUBROUTINE_PARAMS) {

        //Diffuse component
        vec3 dirToLight = normalize(-in_LightToFrag_cameraspace);
        float NdotL     = dot(in_Normal_cameraspace, dirToLight);
        NdotL           = clamp(NdotL, 0.0, 1.0);

        vec3 dirToEye       = normalize(-in_EyeToFrag_cameraspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_cameraspace, halway), 0.0 ,1.0 );

        float lightReach    = in_LightReach_cameraspace;

        float dist          = length(in_LightToFrag_cameraspace);
        float d = max(dist - lightReach, 0);
        // calculate basic attenuation
        float denom = d/lightReach + 1;
        float attenuation = 1 / (denom*denom);
        attenuation = (attenuation - SCE_LightCutoff) / (1 - SCE_LightCutoff);
        attenuation = max(attenuation, 0);

        vec2 light  = vec2(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0)); //specular component

        light *= attenuation;
        return light;
    }

    //Spot light option
    subroutine (SCE_ComputeLightType) vec2 SCE_ComputeSpotLight(LIGHT_SUBROUTINE_PARAMS) {
        //Diffuse component
        vec3 dirToLight     = normalize(-in_LightToFrag_cameraspace);
        vec3 invLightDir    = normalize(-in_LightDirection_cameraspace);
        float NdotL         = dot(in_Normal_cameraspace, dirToLight);
        NdotL               = clamp(NdotL, 0.0, 1.0);

        vec3 dirToEye       = normalize(-in_EyeToFrag_cameraspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_cameraspace, halway), 0.0 ,1.0 );

        float lightReach    = in_LightReach_cameraspace;

        //use very simple fallof approximation to fade spot light with distance
        float dist          = length(in_LightToFrag_cameraspace);
        float d             = mapToRange(0.0, lightReach, 1.0, 0.0, dist);
        float attenuation   = d*d;

        float spotAttenuation = max(dot(dirToLight, invLightDir), 0.0);
        spotAttenuation = pow(spotAttenuation, SCE_SpotAttenuation);

        vec2 light  = vec2(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0)); //specular component

        light *= attenuation * spotAttenuation;
        return light;
    }

    //uniform variable declaration for the light function subroutine
    subroutine uniform SCE_ComputeLightType SCE_ComputeLight;


    in vec3     LightDirection_cameraspace;
    in vec3     LightPosition_cameraspace;
    in float    LightReach_cameraspace;

    out vec4 color;

    uniform sampler2D PositionTex;
    uniform sampler2D DiffuseTex;
    uniform sampler2D NormalTex;

    void main(){

        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 MaterialDiffuseColor   = texture2D(DiffuseTex, uv).xyz;
        vec3 MaterialSpecularColor  = vec3(0.3,0.3,0.3);
        vec3 Normal_cameraspace     = normalize(texture2D(NormalTex, uv).xyz);
        vec3 Position_cameraspace   = texture2D(PositionTex, uv).xyz;

        vec3 LightToFrag_cameraspace = Position_cameraspace - LightPosition_cameraspace;
        vec3 EyeToFrag_cameraspace = Position_cameraspace;

        vec2 lightCol = SCE_ComputeLight(
                    LightDirection_cameraspace,
                    Normal_cameraspace,
                    EyeToFrag_cameraspace,
                    LightToFrag_cameraspace,
                    LightReach_cameraspace
                    );

        color = vec4( //Diffuse
                      (MaterialDiffuseColor * lightCol.x * SCE_LightColor.rgb * SCE_LightColor.a)
                      //Specular
                      + (SCE_LightColor.rgb * lightCol.y * SCE_LightColor.a), 1.0);

        //gamma correction
        color = pow(color, vec4(1.0/2.2));
    }
_}