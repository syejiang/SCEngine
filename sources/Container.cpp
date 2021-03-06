/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.cpp ********/
/**************************************/

#include "../headers/Container.hpp"
#include "../headers/Component.hpp"
#include "../headers/SCEScene.hpp"
#include "../headers/SCEInternal.hpp"

using namespace SCE;
using namespace std;

Container::Container(const string &name, int id)
    : mComponents()
    , mTag(DEFAULT_TAG)
    , mLayer(DEFAULT_LAYER)
    , mName(name)
    , mContainerId(id)
{
//    Internal::Log("New container");
}

Container::~Container()
{
//    Internal::Log("delete container");
//    Internal::Log("delete components");
    for(Component* compo : mComponents)
    {
        delete(compo);
    }
    mComponents.clear();
}

const string& Container::GetTag() const
{
    return mTag;
}

const string& Container::GetLayer() const
{
    return mLayer;
}

void Container::SetTag(const string& tag)
{
    mTag = tag;
}

void Container::SetLayer(const string& layer)
{
    mLayer = layer;
}
const string& Container::GetName() const
{
    return mName;
}

void Container::SetName(const std::string &name)
{
    mName = name;
}
const int& Container::GetContainerId() const
{
    return mContainerId;
}




