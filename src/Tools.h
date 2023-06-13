#pragma once
#include <iostream>
#include <vector>
#include <glm/gtx/quaternion.hpp>

namespace Tools
{
    int GetFileLength(std::ifstream &file)
    {
        int oriPos = file.tellg();
        file.seekg(0, std::ios::end);
        int length = file.tellg();
        file.seekg(oriPos);
        return length;
    }

    void IgnoreLine(std::ifstream &file, int length)
    {
        file.ignore(length, '\n');
    }

    void RemoveNotes(std::string &str)
    {
        str.erase(0, 1);
        str.erase(str.size() - 1, 1);
        //ÆþÍ·È¥Î²
    }

    void ComputeQuatW(glm::quat &quat)
    {
        float t = 1 - quat.x * quat.x - quat.y * quat.y - quat.z * quat.z;
        quat.w = t < 0 ? 0 : -sqrtf(t);
    }

    // void split(std::string Path, std::vector<std::string> pathArr, std::string str)
    // {

    // }

    std::string GetTexureFile(std::string &path, std::string &fileName)
    {
        return path + "/" + fileName;
    }
}