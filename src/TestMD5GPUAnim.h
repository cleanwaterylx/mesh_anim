#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "Tools.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// Ĭ�Ϲ���
struct JointInfo
{
	std::string name; // ����
	int parentID; // ����������
	int flag;  // ��־λ
	int startIndex; // ������֡�����е�����
};
typedef std::vector<JointInfo> JointInfoList;

// �߿�
struct  Bound
{
	glm::vec3 min;//��Сֵ
	glm::vec3 max;//���ֵ
};
typedef std::vector<Bound> BoundList;

//����Ĭ��λ�÷���
struct  BaseFrame
{
	glm::vec3 pos; // ����������ռ��е�λ��
	glm::vec3 orient; // ����������ռ��е���ת
};
typedef std::vector<BaseFrame> BasefFrameList;

//֡����
struct  FrameData
{
	int frameID;// ��ǰ֡����
	std::vector<float> data;// ��ǰ֡����
};
typedef std::vector<FrameData> FrameDataList;

//������λ�úͷ���
struct  SkeletonJoint
{
	SkeletonJoint() :parentID(-1), pos(0) {}
	SkeletonJoint(const BaseFrame& copy) :pos(copy.pos), orient(copy.orient) {}
	int parentID; // ������������
	glm::vec3 pos; // ģ�Ϳռ��е�λ��
	glm::quat orient; //ģ�Ϳռ��е���ת 
};
typedef std::vector<SkeletonJoint> SkeletonJointList;
typedef std::vector<glm::mat4> SkeletonMatrixList;
// ĳһ֡���й�����λ�úͷ���
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
	int numFrame; // ����֡��
	int numJoints; // ������
	int rate; // ֡��
	int numComponents; // ֡���ݸ���ֵ����
	float animDuration; //  ����ʱ��  = numFrame / rate
	float frameDuration; // һ֡��ʱ��  = 1/rate
	float animTime; // �����Ѳ���ʱ��

	JointInfoList jointInfos; // ��������
	BoundList bounds;  // �߿�����
	BasefFrameList baseFrames; // Ĭ��֡��������(����������ռ�)
	FrameDataList frames;  // ����֡����
	FrameSkeletonList skeletons; // ����֡�Ĺ���λ�úͷ���ģ�Ϳռ䣩
	FrameSkeleton animatedSkeleton; // ��ǰʱ���Ĺ���λ�úͷ���ģ�Ϳռ䣩
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
	float fInterpolate = fmodf(animTime, frameDuration) / frameDuration; // ����ط���ֵ������һ֡�������ݶ��ڵ�ǰʱ�̵�ռ��
	InterpolateSkeletons(animatedSkeleton, skeletons[lowFrame], skeletons[highFrame], fInterpolate);
}

void MD5Animation::BuildFrameSkeleton(FrameSkeletonList& skeletons, JointInfoList& jointInfos, BasefFrameList& baseFrames, FrameData& frameData)
{
	FrameSkeleton skeleton;
	for (unsigned int i = 0; i < jointInfos.size();i++)
	{
		unsigned int j = 0;
		///  ����֡���ݣ��õ������ڸ���������ռ��е�λ�úͷ��� 
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

		/// ���������ģ�Ϳռ��е�λ�úͷ��� 
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
