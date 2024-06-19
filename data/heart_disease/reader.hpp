#ifndef CSV_READER_HPP
#define CSV_READER_HPP

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
#include<sstream>
#include<iostream>


inline std::map<std::string, std::vector<std::vector<long double>>> readData(void)
{
	std::map<std::string, std::vector<std::vector<long double>>> result;
	
	result.insert({"inputs", std::vector<std::vector<long double>>()});
	result.insert({"outputs", std::vector<std::vector<long double>>()});
	
	std::ifstream CSVFile;
	
	CSVFile.open("data/heart_disease/heart_statlog_cleveland_hungary_final.csv", std::ios::in);
	
	std::string line;
	unsigned int sampleNo = 0;
	
	std::getline(CSVFile, line); // CSV header
	
	while(std::getline(CSVFile, line))
	{
		std::replace(line.begin(), line.end(), ',', ' ');
		std::istringstream lineStream(line);
		
		long double temp;
		
		if (lineStream >> temp)
		{
			result["inputs"].push_back(std::vector<long double>());
			result["outputs"].push_back(std::vector<long double>());
			
			do
			{
				result["inputs"][sampleNo].push_back(temp);
			}
			while(lineStream >> temp);
			
			assert(result["inputs"][sampleNo].size() >= 2);
			
			result["outputs"][sampleNo].push_back(
				result["inputs"][sampleNo].back()
			);
			
			result["inputs"][sampleNo].pop_back();
			
			sampleNo++;
		}
	}
	
	return result;
}


#endif