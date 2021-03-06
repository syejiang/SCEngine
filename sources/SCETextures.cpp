/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCETextures.cpp*********/
/**************************************/

#include "../headers/SCETextures.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEMetadataParser.hpp"
#include "../headers/SCEInternal.hpp"

//disable unneeded image formats
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STBI_NO_PIC
#define STBI_NO_HDR
#define STBI_NO_PSD
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;

#define DEBUG_TEXTURE_NAME "Textures/Color_Debug.jpg"

#define FORMAT_STR "Format"
#define DDS_STR "DDS"
#define UNCOMPRESSED_STR "UNCOMPRESSED"

#define MIPMAPS_STR "Mipmaps"
#define WRAPPING_STR "Wrapping"
#define CLAMP_STR "Clamp"
#define REPEAT_STR "Repeat"

namespace SCE
{

namespace TextureUtils
{
    //Only here to allow for automatic creation/destruction of data
    struct TexturesData
    {
        TexturesData()
            : loadedTextures(), createdTextures()
        {
            //some setup of stb_image needed
            stbi_set_flip_vertically_on_load(1);
        }

        ~TexturesData()
        {
            Internal::Log("Cleaning up texture system, will delete loaded textures");
            auto beginIt = begin(loadedTextures);
            auto endIt = end(loadedTextures);
            for(auto iterator = beginIt; iterator != endIt; iterator++) {
                Internal::Log("Deleting texture : " + iterator->first);
                glDeleteTextures(1, &(iterator->second));
            }

            for(GLuint texId : createdTextures)
            {
                glDeleteTextures(1, &texId);
            }
        }

        std::map<std::string, GLuint>       loadedTextures;
        std::vector<GLuint>                 createdTextures;
    };

/*      File/Compilation Unit scope variable    */
    TexturesData    texturesData;
    GLuint          debugTexture = GL_INVALID_INDEX;

/*      Utiliatry loading/parsing funtions      */
    SCETextureFormat formatFromString(const string &formatString)
    {
        SCETextureFormat res = DDS_FORMAT;
        if(Tools::ToLowerCase(formatString) == Tools::ToLowerCase(DDS_STR)){
            //res == DDS_FORMAT;
        } else if(Tools::ToLowerCase(formatString) == Tools::ToLowerCase(UNCOMPRESSED_STR)){
            res = UNCOMPRESSED_FORMAT;
        } else {
            Debug::RaiseError("Unknow texture format : " + formatString);
        }

        return res;
    }

    SCETextureWrap wrapModeFromString(const string &wrapString)
    {
        SCETextureWrap res = CLAMP_WRAP;
        if(Tools::ToLowerCase(wrapString) == Tools::ToLowerCase(CLAMP_STR)){
            //res == CLAMP_WRAP;
        } else if(Tools::ToLowerCase(wrapString) == Tools::ToLowerCase(REPEAT_STR)){
            res = REPEAT_WRAP;
        } else {
            Debug::RaiseError("Unknow texture wrap mode: " + wrapString);
        }

        return res;
    }

    GLuint createTextureWithData(uint width, uint height,
                                 GLint internalGPUFormat, GLenum textureFormat, GLenum componentType,
                                 SCETextureWrap wrapMode, bool mipmapsOn,
                                 void* textureData)
    {
        GLuint textureID;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalGPUFormat, width, height, 0, textureFormat,
                     componentType, textureData);

        if(mipmapsOn)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        if(wrapMode == SCETextureWrap::CLAMP_WRAP)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        return textureID;
    }

    //loads texture from disk and create and OpenGL texture with it
    GLuint loadTexture(const string &filename, SCETextureFormat compressionFormat,
                       SCETextureWrap wrapMode, bool mipmapsOn)
    {        
        string fullTexturePath = RESSOURCE_PATH + filename;

        if(!ifstream(fullTexturePath.c_str()))
        {
            fullTexturePath = ENGINE_RESSOURCE_PATH + filename;
        }

        Internal::Log(std::string("TODO : Compression format not yet used : ") + std::to_string(compressionFormat));

        int width, height, nbComponent;
        unsigned char* textureData = stbi_load(fullTexturePath.c_str(),
                  &width,
                  &height,
                  &nbComponent, //will store the actual number of component read
                  0); // force to fill # number of component if not zero

        if(!textureData)
        {
            Debug::RaiseError("Coul not load image at : " + fullTexturePath);
        }

        GLuint textureID = GL_INVALID_INDEX;
        GLint internalGPUFormat;
        GLenum textureFormat, type;
        type = GL_UNSIGNED_BYTE;

        switch(nbComponent)
        {
        case 0 :
            Debug::RaiseError(std::string("texture contains no component !") + std::to_string(nbComponent));
            break;
        case 1 :
            internalGPUFormat = GL_R8;
            textureFormat = GL_R;
            break;
        case 2 :
            internalGPUFormat = GL_RG16;
            textureFormat = GL_RG;
            break;
        case 3 :
            internalGPUFormat = GL_RGB32F;
            textureFormat = GL_RGB;
            break;
        case 4 :
            internalGPUFormat = GL_RGBA32F;
            textureFormat = GL_RGBA;
            break;
        default :
            Debug::RaiseError(std::string("Unexpected number of texture component : ") + std::to_string(nbComponent));
            break;
        }

        if(textureData && nbComponent > 0 && nbComponent <= 4)
        {
            textureID = createTextureWithData(width, height, internalGPUFormat, textureFormat, type,
                                  wrapMode, mipmapsOn, textureData);
        }

        if(textureData)
        {
            stbi_image_free(textureData);
        }

        return textureID;
    }

/*      Texture file loading and parsing    */

    GLuint loadTextureFromMetadata(const string &filename,
                                   SCETextureFormat fallbackFormat,
                                   SCETextureWrap fallbackWrapMode,
                                   bool fallbackMipmaps)
    {
        string fullTexturePath = RESSOURCE_PATH + filename;
        string metadataFile = fullTexturePath + TEXTURE_METADATA_SUFIX;

        if(!ifstream(metadataFile.c_str()))
        {
            fullTexturePath = ENGINE_RESSOURCE_PATH + filename;
            metadataFile = fullTexturePath + TEXTURE_METADATA_SUFIX;
        }

        //load texture metada
        ifstream fileStream(metadataFile.c_str(), ios::in);
        map<string, string> lineData;
        if(fileStream.is_open())
        {
            string currLine;
            string name, formatStr, mipmapStr, wrappingModeStr;

            while (getline(fileStream, currLine))
            {
                lineData = MetadataParser::GetLineData(currLine);
                if(lineData.count("Name") > 0)
                {
                    name = lineData["Name"];
                }
                if(lineData.count(FORMAT_STR) > 0)
                {
                    formatStr = lineData[FORMAT_STR];
                }
                if(lineData.count(MIPMAPS_STR) > 0)
                {
                    mipmapStr = lineData[MIPMAPS_STR];
                }
                if(lineData.count(WRAPPING_STR) > 0)
                {
                    wrappingModeStr = lineData[WRAPPING_STR];
                }
            }
            fileStream.close();

            bool mipmapsOn = mipmapStr != "false";
            //texture format
            SCETextureFormat format = formatFromString(formatStr);
            SCETextureWrap wrapMode = wrapModeFromString(wrappingModeStr);

            return loadTexture(name, format, wrapMode, mipmapsOn);
        }
        else
        {
            //try to open as a raw image directly
            return loadTexture(filename, fallbackFormat,
                               fallbackWrapMode, fallbackMipmaps);
        }
    }

/*      Public interface        */

    GLuint CreateTexture(int width, int height,
                                      vec4 color,
                                      SCETextureFormat compressionFormat, SCETextureWrap wrapMode,
                                      bool mipmapsOn)
    {
        Internal::Log(std::string("TODO : Compression format not yet used : ") + std::to_string(compressionFormat));

        uint channelCount = 4;
        unsigned char *textureData = new unsigned char [width * height * channelCount];
        int xstart, ystart;
        for(int x = 0; x < width; ++x){
            xstart = x * height * channelCount;
            for(int y = 0; y < height; ++y){
                ystart = y * channelCount;
                textureData[xstart + ystart + 0] = Tools::FloatToColorRange(color.x);
                textureData[xstart + ystart + 1] = Tools::FloatToColorRange(color.y);
                textureData[xstart + ystart + 2] = Tools::FloatToColorRange(color.z);
                textureData[xstart + ystart + 3] = Tools::FloatToColorRange(color.w);
            }
        }

        GLuint texId = createTextureWithData(width, height, GL_RGBA32F, GL_RGBA, GL_UNSIGNED_BYTE,
                                             wrapMode, mipmapsOn, textureData);
        texturesData.createdTextures.push_back(texId);

        return texId;
    }

    GLuint LoadTexture(const string& textureName,
                       SCETextureFormat fallbackFormat,
                       SCETextureWrap fallbackWrapMode,
                       bool fallbackMipmaps)
    {
        //texture has already been loaded
        if(texturesData.loadedTextures.count(textureName) > 0)
        {
            Internal::Log("Texture " + textureName + " already loaded, using it directly");
            return texturesData.loadedTextures[textureName];
        }

        GLuint texId = loadTextureFromMetadata(textureName, fallbackFormat, fallbackWrapMode, fallbackMipmaps);
        texturesData.loadedTextures[textureName] = texId;

        return texId;
    }

    void DeleteTexture(GLuint textureId)
    {
        auto itLoaded = find_if(begin(texturesData.loadedTextures),
                          end(texturesData.loadedTextures),
                          [&textureId](std::pair<string, GLuint> entry)
        { return entry.second == textureId; } );

        //texture id was found in loaded textures
        if(itLoaded != end(texturesData.loadedTextures))
        {
            Internal::Log("Delete texture : " + itLoaded->first);
            glDeleteTextures(1, &(itLoaded->second));
            texturesData.loadedTextures.erase(itLoaded);
        }

        auto itCreated = find(begin(texturesData.createdTextures),
                       end(texturesData.createdTextures),
                       textureId);

        //texture id was found in created textures
        if(itCreated != end(texturesData.createdTextures))
        {
            glDeleteTextures(1, &textureId);
            texturesData.createdTextures.erase(itCreated);
        }
    }

    void BindTexture(GLuint textureId, GLuint textureUnit, GLuint samplerUniformId)
    {        
        // Bind the texture to a texture Unit
        glActiveTexture(GL_TEXTURE0 + textureUnit);

        if(debugTexture != GL_INVALID_INDEX)//use debug texture
        {
            glBindTexture(GL_TEXTURE_2D, debugTexture);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, textureId);
        }

        // Set the sampler uniform to the texture unit
        glUniform1i(samplerUniformId, textureUnit);
    }

    void BindSafeTexture(GLuint textureId, GLuint textureUnit, GLuint samplerUniformId)
    {
        // Bind the texture to a texture Unit
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Set the sampler uniform to the texture unit
        glUniform1i(samplerUniformId, textureUnit);
    }

#ifdef SCE_DEBUG_ENGINE
    void EnableDebugTexture()
    {
        debugTexture = LoadTexture(DEBUG_TEXTURE_NAME);
    }

    void DisableDebugTexture()
    {
        debugTexture = GL_INVALID_INDEX;
    }

    bool ToggleDebugTexture()
    {
        if(debugTexture == GL_INVALID_INDEX)
        {
            EnableDebugTexture();
            return true;
        }
        else
        {
            DisableDebugTexture();
            return false;
        }
    }
#endif

}

}











