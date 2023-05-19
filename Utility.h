#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <vector>
#include <fstream>	
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
    #include <direct.h>
    #define GetCurrentDir _getcwd
    #define CREATE_DIRECTORY std::experimental::filesystem::create_directory
#elif defined(__APPLE__)
	#include <unistd.h>
	#define GetCurrentDir getcwd
	#define CREATE_DIRECTORY boost::filesystem::create_directory
#else
    #include <experimental/filesystem>
    #define CREATE_DIRECTORY std::experimental::filesystem::create_directory
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#include <QGroupBox>
#include <QCheckBox>
#include <QFrame>
#include <cmath>

namespace InsermLibrary
{
	enum FileExt { NO_EXT = -1, TRC, EEG_ELAN, ENT_ELAN, POS_ELAN, POS_DS_ELAN, SM0_ELAN, SM250_ELAN, SM500_ELAN, SM1000_ELAN, SM2500_ELAN, SM5000_ELAN, EDF, BRAINVISION };
    enum class SmoothingWindow { SM0, SM250, SM500, SM1000, SM2500, SM5000 };

	/********************************************************************/
	/*	vector<string> v = split<string>("Hello, there; World", ";,");	*/
	/********************************************************************/
	template<typename T>
	std::vector<T> split(const T & str, const T & delimiters)
	{
		std::vector<T> v;
		typename T::size_type start = 0;
		auto pos = str.find_first_of(delimiters, start);
		while (pos != T::npos) {
			if (pos != start) // ignore empty tokens
				v.emplace_back(str, start, pos - start);
			start = pos + 1;
			pos = str.find_first_of(delimiters, start);
		}
		if (start < str.length()) // ignore trailing delimiter
			v.emplace_back(str, start, str.length() - start); // add what's left of the string
		return v;
	}

	template<typename T>
	void deleteAndNullify1D(T& pointer)
	{
		if (pointer != nullptr)
		{
			delete pointer;
			pointer = nullptr;
		}
	}

	template<typename T>
	T stdDeviation(T* myArray, int sizeFirstDim, int mean)
	{
		T stdTempValue = 0;

		for (int i = 0; i < sizeFirstDim; i++)
		{
			stdTempValue += (myArray[i] - mean) * (myArray[i] - mean);
		}

		return sqrt(stdTempValue / (sizeFirstDim - 1));
	}

	template<typename T>
	T mean1DArray(T* myArray, int sizeFirstDim)
	{
		T sumValues = 0;
		for (int i = 0; i < sizeFirstDim; i++)
		{
			sumValues += myArray[i];
		}
		return (sumValues / sizeFirstDim);
	}

	template<typename T>
	T mean2DArray(T** myArray, int sizeFirstDim, int posSecondDim)
	{
		T sumValues = 0;
		for (int i = 0; i < sizeFirstDim; i++)
		{
			sumValues += myArray[i][posSecondDim];
		}
		return (sumValues / sizeFirstDim);
	}

	template<class T>
	using vec1 = std::vector<T>; /**< templated std vector alias */

	template<class T>
	using vec2 = std::vector<vec1<T>>; /**< templated std vector of std vector alias */

	template<class T>
	using vec3 = std::vector<vec2<T>>; /**< templated std vector of std vector of std vector alias */

    template<class T>
    using vec4 = std::vector<vec3<T>>; /**< templated std vector of std vector of std vector alias */

	std::vector<std::string> readTxtFile(std::string path);

	void saveTxtFile(std::vector<QString> data, std::string pathFile);
	
	std::string GetCurrentWorkingDir();

	template<typename T> 
	void OutputArrayToCsv1D(T* array, int sizeArray, std::string filePath)
	{
        std::ofstream fileStream(filePath, std::ios::out);
		for (int i = 0; i < sizeArray; i++)
			fileStream << array[i] << ";" << std::endl;
		fileStream.close();
	}
}

#endif
