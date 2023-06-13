#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <glad/glad.h>
#include "Tools.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "stb_image.h"
#include "Shader.h"
#include "TestMD5GPUAnim.h"
using namespace std;
struct Joint
{
	string name;	  // 骨骼名称
	int parent_ID;	  // 父骨骼在骨骼层次结构中的索引
	glm::vec3 pos;	  // 初始姿势时骨骼在模型空间中的位置
	glm::quat orient; // 初始姿势时骨骼坐标空间相对模型空间的旋转
};
typedef vector<Joint> JointList;

struct Vertex
{
	glm::vec3 pos;		   // 顶点在模型空间中的位置
	glm::vec2 texcoord;	   // 纹理坐标
	glm::vec4 boneWeights; // 所关联骨骼的权重
	glm::vec4 boneIndexs;  // 所关联骨骼的索引
	int startWeight;	   // 所关联权重的起始索引
	int weightCount;	   // 所关联的权重总数
};
typedef vector<Vertex> VertexList;

struct Weight
{
	int joint_ID;  // 与该权重关联的骨骼在骨骼层次结构中的索引
	float bias;	   // 权重占比
	glm::vec3 pos; // 权重在所关联骨骼坐标空间中的位置
};
typedef vector<Weight> WeightList;

typedef vector<glm::mat4> MatrixList;

typedef vector<unsigned int> IndexBuffer;
struct Mesh
{
	string shader;				// 纹理
	unsigned int texID;			// 纹理缓冲对象
	unsigned int VAO, VBO, EBO; // 顶点数组对象，顶点缓冲对象，索引缓冲对象
	VertexList verts;			// 顶点数组（这里用数组来表示集合，数据结构是vertor）
	WeightList weights;			// 权重数组
	IndexBuffer indexBuffer;	// 索引数组
};
typedef vector<Mesh> MeshList;

class TestMD5
{
public:
	TestMD5();
	~TestMD5();
	bool LoadModel(string &path);
	unsigned int LoadTexture(string &path);
	void CreateVertexBuffer(Mesh &mesh);
	void ComputeMatrix(Mesh &mesh, const FrameSkeleton &skeleton);
	void BuildBindPose(JointList &jointList);
	void PrepareMesh(Mesh &mesh);
	void Update(float deltaTime);
	void Render(Shader shader);
	MD5Animation animation; // 动画类 后面会详细讲到
private:
	int numJonints;		 // 骨骼数量
	int numMeshes;		 // mesh数量
	JointList jointList; // 骨骼数组
	MeshList meshList;	 // mesh数组
	MatrixList inverseBindPose;
	MatrixList animatedBones;
};

bool TestMD5::LoadModel(string &path)
{
	// string str(absolutePath);
	// path = str + path;
	if (path.substr(path.size() - 8, 8) != ".md5mesh")
	{
		cout << "Only Load md5mesh Model" << path << endl;
		return false;
	}
	std::ifstream file(path);
	if (!file.is_open())
	{
		cout << "oepn file erro " << path << endl;
		return false;
	}
	string param;
	string junk;

	jointList.clear();
	meshList.clear();
	int length = Tools::GetFileLength(file);
	file >> param;
	while (!file.eof())
	{
		if (param == "MD5Version" || param == "commandline")
		{
			Tools::IgnoreLine(file, length);
		}
		if (param == "numJoints")
		{
			file >> numJonints;
			jointList.reserve(numJonints);
		}
		else if (param == "numMeshes")
		{
			file >> numMeshes;
			meshList.reserve(numMeshes);
		}
		else if (param == "joints")
		{
			Joint joint;
			file >> junk;
			for (int i = 0; i < numJonints; i++)
			{
				file >> joint.name >> joint.parent_ID >> junk >> joint.pos.x >> joint.pos.y >> joint.pos.z >> junk >> junk >> joint.orient.x >> joint.orient.y >> joint.orient.z >> junk;
				Tools::IgnoreLine(file, length);
				Tools::RemoveNotes(joint.name);
				Tools::ComputeQuatW(joint.orient);
				jointList.push_back(joint);
			}
			file >> junk;
			BuildBindPose(jointList);
		}
		else if (param == "mesh")
		{
			Mesh mesh;
			int numVert, numTris, numWeight;
			file >> junk;
			file >> param;
			while (param != "}")
			{
				if (param == "shader")
				{
					file >> mesh.shader;
					Tools::RemoveNotes(mesh.shader);
					std::string oripath = "../Model";
					string texPath = Tools::GetTexureFile(oripath, mesh.shader);
					if (texPath.substr(texPath.size() - 4, 4) != ".tga")
					{
						texPath += ".tga";
					}
					mesh.texID = LoadTexture(texPath); // 加载纹理并生成纹理缓冲对象
					Tools::IgnoreLine(file, length);
				}
				else if (param == "numverts")
				{
					file >> numVert;
					Tools::IgnoreLine(file, length);
					for (int i = 0; i < numVert; i++)
					{
						Vertex vert;
						file >> junk >> junk >> junk >> vert.texcoord.x >> vert.texcoord.y >> junk >> vert.startWeight >> vert.weightCount;
						Tools::IgnoreLine(file, length);
						mesh.verts.push_back(vert);
					}
				}
				else if (param == "numtris")
				{
					file >> numTris;
					Tools::IgnoreLine(file, length);
					for (int i = 0; i < numTris; i++)
					{
						int indices[3];
						file >> junk >> junk >> indices[0] >> indices[1] >> indices[2];
						Tools::IgnoreLine(file, length);
						mesh.indexBuffer.push_back((GLint)indices[0]);
						mesh.indexBuffer.push_back((GLint)indices[1]);
						mesh.indexBuffer.push_back((GLint)indices[2]);
					}
				}
				else if (param == "numweights")
				{
					file >> numWeight;
					Tools::IgnoreLine(file, length);
					for (int i = 0; i < numWeight; i++)
					{
						Weight weight;
						file >> junk >> junk >> weight.joint_ID >> weight.bias >> junk >> weight.pos.x >> weight.pos.y >> weight.pos.z;
						Tools::IgnoreLine(file, length);
						mesh.weights.push_back(weight);
					}
				}
				else
				{
					Tools::IgnoreLine(file, length);
				}
				file >> param;
			}
			PrepareMesh(mesh);
			CreateVertexBuffer(mesh);
			meshList.push_back(mesh);
		}
		file >> param;
	}
	return jointList.size() == numJonints && meshList.size() == numMeshes;
}

void TestMD5::BuildBindPose(JointList &jointList)
{
	inverseBindPose.clear();

	JointList::const_iterator iter = jointList.begin();
	while (iter != jointList.end())
	{
		const Joint &joint = (*iter);
		glm::mat4 boneMatrix(1.0f);
		glm::mat4 boneTranstion = glm::translate(boneMatrix, joint.pos);
		glm::mat4 boneRotate = glm::toMat4(joint.orient);
		// 针对一个空间坐标系 P，对该空间进行一个旋转 M ，再进行一个平移 T， 得到新的空间坐标系 C， 对于空间C内一点 V（x,y,z） ,V在P空间内的坐标为
		//  V' = T * M  * V
		boneMatrix = boneTranstion * boneRotate;				// 骨骼空间到 模型空间的变换矩阵
		glm::mat4 inverseBoneMatrix = glm::inverse(boneMatrix); // 模型空间到 骨骼空间 的矩阵
		inverseBindPose.push_back(inverseBoneMatrix);
		iter++;
	}
}

void TestMD5::PrepareMesh(Mesh &mesh)
{
	for (unsigned int i = 0; i < mesh.verts.size(); i++)
	{
		Vertex &vert = mesh.verts[i];
		vert.pos = glm::vec3(0);
		vert.boneIndexs = glm::vec4(0);
		vert.boneWeights = glm::vec4(0);

		for (int j = 0; j < vert.weightCount; j++)
		{
			Weight &weight = mesh.weights[vert.startWeight + j];
			Joint &joint = jointList[weight.joint_ID];
			glm::vec3 rotPos = joint.orient * weight.pos;
			vert.pos += (joint.pos + rotPos) * weight.bias;
			vert.boneWeights[j] = weight.bias;
			vert.boneIndexs[j] = weight.joint_ID;
		}
	}
}

unsigned int TestMD5::LoadTexture(string &path)
{
	int width, height, channel;
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &channel, NULL);
	if (!data)
	{
		cout << "load texture fail path:" << path << endl;
		return 0;
	}

	int format;
	format = channel == 3 ? GL_RGB : GL_RGBA;
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	return texture;
}

void TestMD5::ComputeMatrix(Mesh &mesh, const FrameSkeleton &skeleton)
{
	animatedBones.clear();
	for (int i = 0; i < numJonints; i++)
	{
		// mat = bone.trans * bone.rot  = skeleton.boneMatrixs[i] * inverseBindPose
		glm::mat4 matrix = skeleton.boneMatrixs[i] * inverseBindPose[i];
		animatedBones.push_back(matrix);
	}
}

void TestMD5::CreateVertexBuffer(Mesh &mesh)
{
	glGenVertexArrays(1, &(mesh.VAO));
	glGenBuffers(1, &(mesh.VBO));
	glGenBuffers(1, &(mesh.EBO));
	glBindVertexArray(mesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh.verts.size() * sizeof(Vertex), &(mesh.verts[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, boneWeights));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, boneIndexs));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer.size() * sizeof(unsigned int), &(mesh.indexBuffer[0]), GL_STATIC_DRAW);
	glBindVertexArray(0);
}

void TestMD5::Update(float deltaTime)
{
	animation.Update(deltaTime);
	const FrameSkeleton &skeleton = animation.GetSkeleton();
	animatedBones.clear();
	for (int i = 0; i < numJonints; i++)
	{
		glm::mat4 matrix = skeleton.boneMatrixs[i] * inverseBindPose[i]; // 当前姿势的骨骼矩阵 * 绑定姿势的逆骨骼矩阵
		animatedBones.push_back(matrix);
	}
}

void TestMD5::Render(Shader shader)
{
	shader.setMat4List("boneMatrixs", animatedBones.size(), animatedBones[0]);
	for (unsigned int i = 0; i < meshList.size(); i++)
	{
		Mesh mesh = meshList[i];
		glBindVertexArray(mesh.VAO);
		shader.setInt("diffTex", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh.texID);
		glDrawElements(GL_TRIANGLES, mesh.indexBuffer.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

TestMD5::TestMD5()
{
}

TestMD5::~TestMD5()
{
	MeshList::iterator iter = meshList.begin();
	while (iter != meshList.end())
	{
		Mesh &mesh = *iter;
		glDeleteBuffers(1, &mesh.VBO);
		glDeleteBuffers(1, &mesh.EBO);
	}
}