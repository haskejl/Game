/*
 * ModelLoader.cpp
 *
 *  Created on: May 5, 2019
 *      Author: user1
 */

#include "ModelLoader.h"

ModelLoader::ModelLoader() {

}

ModelLoader::~ModelLoader() {
	// TODO Auto-generated destructor stub
}

/**
 * Loads the model in from a Blender output Collada file.
 *
 * @param fileName A string representing the file name.
 */
void ModelLoader::loadModel(std::string &fileName, Model &model)
{
	//Opening of block regular expressions
	std::regex materialBlockStart("^\\s+<library_effects>");
	std::regex vertexBlockStart("^\\s+<source.+positions.+>");
	std::regex triangleBlockStart("^\\s+<triangles.+>");

	//Closing of block regular expressions
	std::regex materialBlockEnd("^\\s+</library_effects>");
	std::regex vertexBlockEnd("^\\s+</source>");
	std::regex triangleBlockEnd("^\\s+</triangles>");

	//Misc regular expressions
	std::regex scaleRegex(".+<scale.+");

	std::string line;

	std::ifstream file(fileName);

	if(file.is_open())
	{
		std::cout << fileName << '\n';
		int state = 0; //default 0, material 1, vertex 2, triangle 3

		//load each line for processing
		while(std::getline(file, line))
		{
			std::cout << line << '\n';
			//Determine if line is a material
			if(std::regex_match(line, materialBlockStart)) state = 1;
			//Determine if line is a vertex
			else if(std::regex_match(line, vertexBlockStart)) state = 2;
			//Determine if line is a triangle
			else if(std::regex_match(line, triangleBlockStart)) state = 3;

			//Process chunk of lines
			switch(state)
			{
			case 1: //material
				do
				{
					removeExponents(line);
					processMaterial(line, model);
				}while(std::getline(file, line) && !regex_match(line, materialBlockEnd));
				state = 0;
				break;
			case 2: //vertex
				do
				{
					removeExponents(line);
					processVertex(line, model);
				}while(std::getline(file, line) && !regex_match(line, vertexBlockEnd));
				state = 0;
				break;
			case 3:// triangle
				do
				{
					std::cout << line << '\n';
					processTriangle(line, model);
				}while(std::getline(file, line) && !regex_match(line, triangleBlockEnd));
				state = 0;
				break;
			default:
				break;
			}
		}
		file.close();
	}
	else if(file.fail())
	{
		std::cout << "Failed to open file: " << fileName << '\n';
	}
	model.printModel();
}//end loadModel(std::string) method

/**
 * Processes the materials block
 *
 * @param line
 */
void ModelLoader::processMaterial(std::string &line, Model &model)
{
	std::regex materialRegex(".+<effect.+>");
	std::regex matNameRegex("[a-zA-Z0-9]+[-]{1}effect");
	std::regex emissionRegex(".+<color.+emission.+>.+<.+>");
	std::regex ambientRegex(".+<color.+ambient.+>.+<.+>");
	std::regex diffuseRegex(".+<color.+diffuse.+>.+<.+>");
	std::regex specularRegex(".+<color.+specular.+>.+<.+>");

	if(std::regex_match(line, materialRegex))
	{
		std::smatch matches;
		std::regex_search(line, matches, matNameRegex);
		std::string str = matches[0];
		str = std::regex_replace(str, std::regex("effect"), "material");
		model.addMaterial(str);

		//rename to material instead of effect
	}
	else if(std::regex_match(line, emissionRegex))
	{
		Vec4D<GLfloat> tVec;
		getFloat4D(line, tVec);
		model.matSetEmission(tVec);
	}
	else if(std::regex_match(line, ambientRegex))
	{
		Vec4D<GLfloat> tVec;
		getFloat4D(line, tVec);
		model.matSetAmbient(tVec);
	}
	else if(std::regex_match(line, diffuseRegex))
	{
		Vec4D<GLfloat> tVec;
		getFloat4D(line, tVec);
		model.matSetDiffuse(tVec);
	}
	else if(std::regex_match(line, specularRegex))
	{
		Vec4D<GLfloat> tVec;
		getFloat4D(line, tVec);
		model.matSetSpecular(tVec);
	}
}

void ModelLoader::processVertex(std::string &line, Model &model)
{
	std::regex vertexRegex(".+<float_array.+mesh-positions-array.+>.+<.+>");
	std::regex floatRegex("[+-]*[0-9]+[.]{0,1}[0-9]*");

	std::string temp[10000];
	if(regex_match(line, vertexRegex))
	{
		std::cout << "Started vertices.\n";
		int numElements = removeTags(line, floatRegex, temp);
		for(int i=0; i<numElements; i+=3)
		{
			model.addVertex(Vec3D<GLfloat>(std::strtof(temp[i].c_str(), NULL), std::strtof(temp[i+1].c_str(), NULL), std::strtof(temp[i+2].c_str(), NULL)));
		}
		std::cout << "Completed vertices.\n";
	}
}

void ModelLoader::processTriangle(std::string &line, Model &model)
{
	std::regex triangleMatRegex("^\\s+<triangles.+material.+>");
	std::regex triangleMatNameRegex("[a-zA-Z0-9]+[-]{1}material");
	std::regex triangleRegex("^\\s+<p>");
	std::regex intRegex("[0-9]+");

	std::cout << "Started triangles.\n";
	//find each triangle material
	if(std::regex_match(line, triangleMatRegex))
	{
		std::smatch matches;
		std::regex_search(line, matches, triangleMatNameRegex);
		model.setTriangleMaterial(matches[0]);
	}
	//Check if the line contains the triangles
	else if(std::regex_search(line, triangleRegex, std::regex_constants::match_any))
	{
		std::string temp[10000];
		int numElements = removeTags(line, intRegex, temp);
		//skip every other number
		for(int i = 0; i < numElements && !temp[i].empty(); i+=6)
		{
			model.addTriangle(Vec3D<int>(std::stoi(temp[i].c_str(), NULL), std::stoi(temp[i+2].c_str(), NULL), std::stoi(temp[i+4].c_str(), NULL)));
		}
	}
	std::cout << "Completed triangles.\n";
}

void ModelLoader::getFloat4D(std::string &line, Vec4D<GLfloat> &v)
{
	std::regex floatRegex("[+-]*[0-9]+[.]{0,1}[0-9]*");

	std::string temp[4];
	removeTags(line, floatRegex, temp);
	v = Vec4D<GLfloat>(std::strtof(temp[0].c_str(), NULL), std::strtof(temp[1].c_str(), NULL), std::strtof(temp[2].c_str(), NULL), std::strtof(temp[3].c_str(), NULL));
}

/**
 * Searches a line and returns an array of strings
 */
int ModelLoader::removeTags(std::string &line, std::regex &numRegex, std::string *returnArray)
{
	std::regex stripRegex("^\\s+<(p|float_array.+\")>");
	std::regex stripRegex1("</.+>");

	//strip the end </...> first to allow for searching for <...>
	//Leaves only the numbers, space delimited
	line = std::regex_replace(line, stripRegex, "", std::regex_constants::format_first_only);

	std::cout << line << '\n';
	std::smatch matches;
	int i = 0;
	while(regex_search(line, matches, numRegex))
	{
		returnArray[i++] = matches[0];
		line = std::regex_replace(line, numRegex, "", std::regex_constants::format_first_only);
	}
	return i;
}//end removeTags(std::string line, std::regex, std::regex)

/**
 * Scales the model from the Collada file.
 */
void ModelLoader::scaleModel()
{
	/*for(int i = 0; i < numVertices; i++)
	{
		v[i].coord.scale(scale);
	}*/
}

/**
 * Removes any exponents from the floating point numbers in the Collada file.
 *
 * @param line
 */
void ModelLoader::removeExponents(std::string &line)
{
	std::regex stripRegex("[+-]*[0-9]+[.]{0,1}[0-9]*[e]{1}[-]{1}[0-9]+");

	line = std::regex_replace(line, stripRegex, "0");
}
