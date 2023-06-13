#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "Tools.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// 默认骨骼
struct JointInfo
{
	std::string name; // 名称
	int parentID; // 父骨骼索引
	int flag;  // 标志位
	int startIndex; // 骨骼在帧数据中的索引
};
typedef std::vector<JointInfo> JointInfoList;

// 边框
struct  Bound
{
	glm::vec3 min;//最小值
	glm::vec3 max;//最大值
};
typedef std::vector<Bound> BoundList;

//骨骼默认位置方向
struct  BaseFrame
{
	glm::vec3 pos; // 父骨骼坐标空间中的位置
	glm::vec3 orient; // 父骨骼坐标空间中的旋转
};
typedef std::vector<BaseFrame> BasefFrameList;

//帧数据
struct  FrameData
{
	int frameID;// 当前帧索引
	std::vector<float> data;// 当前帧数据
};
typedef std::vector<FrameData> FrameDataList;

//骨骼的位置和方向
struct  SkeletonJoint
{
	SkeletonJoint() :parentID(-1), pos(0) {}
	SkeletonJoint(const BaseFrame& copy) :pos(copy.pos), orient(copy.orient) {}
	int parentID; // 父骨骼的索引
	glm::vec3 pos; // 模型空间中的位置
	glm::quat orient; //模型空间中的旋转 
};
typedef std::vector<SkeletonJoint> SkeletonJointList;
typedef std::vector<glm::mat4> SkeletonMatrixList;
// 某一帧所有骨骼的位置和方向
struct  FrameSkeleton
{
	SkeletonMatrixList boneMatrixs;
	SkeletonJointList jointList;
};
typedef std::vector<FrameSkeleton> FrameSkeletonList;

class MD5Animation
{
public:
	MD5Animation();
	~MD5Animation();
	bool LoadAnimation(std::string& path);
	void Update(float deltaTime);
	const FrameSkeleton& GetSkeleton() const
	{
		return animatedSkeleton;
	};
private:
	int numFrame; // 动画帧数
	int numJoints; // 骨骼数
	int rate; // 帧率
	int numComponents; // 帧数据浮点值数量
	float animDuration; //  动画时长  = numFrame / rate
	float frameDuration; // 一帧的时间  = 1/rate
	float animTime; // 动画已播放时长

	JointInfoList jointInfos; // 骨骼数据
	BoundList bounds;  // 边框数据
	BasefFrameList baseFrames; // 默认帧骨骼数据(父骨骼坐标空间)
	FrameDataList frames;  // 所有帧数据
	FrameSkeletonList skeletons; // 所有帧的骨骼位置和方向（模型空间）
	FrameSkeleton animatedSkeleton; // 当前时间点的骨骼位置和方向（模型空间）
	void BuildFrameSkeleton(FrameSkeletonList& skeletons, JointInfoList& jointInfos, BasefFrameList& baseFrames, FrameData& frameData);
	void InterpolateSkeletons(FrameSkeleton& finalSkeleton, FrameSkeleton& skeleton0, FrameSkeleton& skeleton1, float fInterpolate);
};

bool MD5Animation::LoadAnimation(std::string& path)
{
	if (path.substr(path.size() - 8, 8) != ".md5anim")
	{
		std::cout << "Only Load md5Anim Animation" << path << std::endl;
		return false;
	}
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cout << "open file erro " << path << std::endl;
		return false;
	}
	std::string param;
	std::string junk;
	jointInfos.clear();
	bounds.clear();
	baseFrames.clear();
	frames.clear();
	animatedSkeleton.jointList.clear();
	numFrame = 0;
	int length = Tools::GetFileLength(file);
	file >> param;
	while (!file.eof())
	{
		if (param == "MD5Version" || param == "commandline")
		{
			Tools::IgnoreLine(file, length);
		}
		else if (param == "numFrames")
		{
			file >> numFrame;
			Tools::IgnoreLine(file, length);
		}
		else if (param == "numJoints")
		{
			file >> numJoints;
			Tools::IgnoreLine(file, length);
		}
		else if (param == "frameRate")
		{
			file >> rate;
			Tools::IgnoreLine(file, length);
		}
		else if (param == "numAnimatedComponents")
		{
			file >> numComponents;
			Tools::IgnoreLine(file, length);
		}
		else  if (param == "hierarchy")
		{
			file >> junk;
			for (int i = 0;i < numJoints;i++)
			{
				JointInfo jointInfo;
				file >> jointInfo.name >> jointInfo.parentID >> jointInfo.flag >> jointInfo.startIndex;
				Tools::IgnoreLine(file, length);
				Tools::RemoveNotes(jointInfo.name);
				jointInfos.push_back(jointInfo);
			}
			file >> junk;
		}
		else if (param == "bounds")
		{
			file >> junk;
			Tools::IgnoreLine(file, length);
			for (int i = 0;i < numFrame;i++)
			{
				Bound bound;
				file >> junk >> bound.min.x >> bound.min.y >> bound.min.z >> junk >> junk
					>> bound.max.x >> bound.max.y >> bound.max.z;
				Tools::IgnoreLine(file, length);
				bounds.push_back(bound);
			}
			file >> junk;
			Tools::IgnoreLine(file, length);
		}
		else if (param == "baseframe")
		{
			file >> junk;
			Tools::IgnoreLine(file, length);
			for (int i = 0; i < numJoints;i++)
			{
				BaseFrame baseFrame;
				file >> junk >> baseFrame.pos.x >> baseFrame.pos.y >> baseFrame.pos.z >> junk >> junk
					>> baseFrame.orient.x >> baseFrame.orient.y >> baseFrame.orient.z;
				Tools::IgnoreLine(file, length);
				baseFrames.push_back(baseFrame);
			}
			file >> junk;
			Tools::IgnoreLine(file, length);
		}

		else if (param == "frame")
		{
			FrameData frame;
			file >> frame.frameID >> junk;
			Tools::IgnoreLine(file, length);

			for (int i = 0;i < numComponents;i++)
			{
				float iData;
				file >> iData;
				frame.data.push_back(iData);
			}
			file >> junk;
			Tools::IgnoreLine(file, length);
			frames.push_back(frame);
			BuildFrameSkeleton(skeletons, jointInfos, baseFrames, frame);
		}
		file >> param;
	}
	for (int i = 0;i < numJoints;i++)
	{
		SkeletonJoint joint;
		glm::mat4 matrix(1.0f);
		animatedSkeleton.jointList.push_back(joint);
		animatedSkeleton.boneMatrixs.push_back(matrix);
	}
	frameDuration = 1.0f / rate;
	animDuration = frameDuration * numFrame;
	animTime = 0;
	return true;
}

void  MD5Animation::Update(float deltaTime)
{
	animTime += deltaTime;
	while (animTime > animDuration) animTime -= animDuration;
	while (animTime < 0) animTime += animDuration;
	float framePro = animTime * rate;
	int lowFrame = (int)floorf(framePro);
	int highFrame = (int)ceilf(framePro);
	lowFrame = lowFrame % numFrame;
	highFrame = highFrame % numFrame;
	float fInterpolate = fmodf(animTime, frameDuration) / frameDuration; // 这个地方插值就是下一帧骨骼数据对于当前时刻的占比
	InterpolateSkeletons(animatedSkeleton, skeletons[lowFrame], skeletons[highFrame], fInterpolate);
}

void MD5Animation::BuildFrameSkeleton(FrameSkeletonList& skeletons, JointInfoList& jointInfos, BasefFrameList& baseFrames, FrameData& frameData)
{
	FrameSkeleton skeleton;
	for (unsigned int i = 0; i < jointInfos.size();i++)
	{
		unsigned int j = 0;
		///  解析帧数据，得到骨骼在父骨骼坐标空间中的位置和方向 
		const JointInfo& jointInfo = jointInfos[i];
		SkeletonJoint animatedJoint = baseFrames[i];
		animatedJoint.parentID = jointInfo.parentID;
		if (jointInfo.flag & 1)
		{
			animatedJoint.pos.x = frameData.data[jointInfo.startIndex + j];
			j++;
		}
		if (jointInfo.flag & 2)
		{
			animatedJoint.pos.y = frameData.data[jointInfo.startIndex + j];
			j++;
		}
		if (jointInfo.flag & 4)
		{
			animatedJoint.pos.z = frameData.data[jointInfo.startIndex + j];
			j++;
		}
		if (jointInfo.flag & 8)
		{
			animatedJoint.orient.x = frameData.data[jointInfo.startIndex + j];
			j++;
		}
		if (jointInfo.flag && 16)
		{
			animatedJoint.orient.y = frameData.data[jointInfo.startIndex + j];
			j++;
		}
		if (jointInfo.flag && 32)
		{
			animatedJoint.orient.z = frameData.data[jointInfo.startIndex + j];
			j++;
		}

		/// 计算骨骼在模型空间中的位置和方向 
		Tools::ComputeQuatW(animatedJoint.orient);
		if (animatedJoint.parentID >= 0)
		{
			SkeletonJoint& parentJoint = skeleton.jointList[animatedJoint.parentID];
			glm::vec3 rotPos = parentJoint.orient * animatedJoint.pos;
			animatedJoint.pos = parentJoint.pos + rotPos;
			animatedJoint.orient = parentJoint.orient * animatedJoint.orient;
			animatedJoint.orient = glm::normalize(animatedJoint.orient);
		}
		skeleton.jointList.push_back(animatedJoint);
	}
	skeletons.push_back(skeleton);
}

void MD5Animation::InterpolateSkeletons(FrameSkeleton& finalSkeleton, FrameSkeleton& skeleton0, FrameSkeleton& skeleton1, float fInterpolate)
{
	for (int i = 0; i < numJoints; i++)
	{
		SkeletonJoint& joint0 = skeleton0.jointList[i];
		SkeletonJoint& joint1 = skeleton1.jointList[i];
		SkeletonJoint& finalJoint = finalSkeleton.jointList[i];
		glm::mat4 finalMatrix(1.0f);
		finalJoint.parentID = joint0.parentID;
		finalJoint.pos = joint0.pos * (1 - fInterpolate) + joint1.pos * fInterpolate;
		finalJoint.orient = glm::mix(joint0.orient, joint1.orient, fInterpolate);
		finalMatrix = glm::translate(finalMatrix, finalJoint.pos)* glm::toMat4(finalJoint.orient);
		finalSkeleton.boneMatrixs[i] = finalMatrix;
	}
}
MD5Animation::MD5Animation()
{
}

MD5Animation::~MD5Animation()
{
}
