#pragma once

#include <vector>
#include <cassert>
#include <iostream>


struct Weight
{
	long double m_value;
	
	Weight(long double p_value = 1) 
	: m_value(p_value)
	{}
};

struct ModelParams
{
	unsigned int m_noOfInputs;
	unsigned int m_noOfLayers;
	long double m_learningRate;
	std::vector<unsigned int> m_noOfNeuronsList;
	std::vector<std::vector<std::vector<Weight>>> m_weightList;
	std::vector<std::vector<long double>> m_biasList;
	
	ModelParams(
		unsigned int p_noOfInputs = 0, 
		unsigned int p_noOfLayers = 0, 
		long double p_learningRate = 0.0, 
		std::vector<unsigned int> p_noOfNeuronsList = {}, 
		long double p_initialWeight = 1,
		long double p_initialBias = 1
	) : 
		m_noOfInputs(p_noOfInputs), 
		m_noOfLayers(p_noOfLayers), 
		m_learningRate(p_learningRate)
	{
		assert(p_noOfNeuronsList.size() == m_noOfLayers);
		
		m_noOfNeuronsList.push_back(m_noOfInputs);
		
		for (unsigned int& noOfNeurons : p_noOfNeuronsList)
		{
			m_noOfNeuronsList.push_back(noOfNeurons);
		}
		
		for(
			unsigned int layerNo = 1; 
			layerNo <= m_noOfLayers; 
			layerNo++
		)
		{
			m_weightList.push_back(
				std::vector<std::vector<Weight>>()
			);
			
			m_biasList.push_back(
				std::vector<long double>(
					m_noOfNeuronsList[layerNo], p_initialBias
				)
			);
			
			for(
				unsigned int neuronNo = 0; 
				neuronNo < m_noOfNeuronsList[layerNo-1]; 
				neuronNo++
			)
			{
				m_weightList[layerNo-1].push_back(std::vector<Weight>(m_noOfNeuronsList[layerNo], p_initialWeight));
			}
		}
	}
};
template <typename T> constexpr bool isVector(const std::vector<T>&)
{
	return true;
};

template <typename T> constexpr bool isVector(const T&)
{
	return false;
};

template <typename T> void printVector(std::ostream& os, const std::vector<T>& vec, const int tabCount = 0)
{
	T temp;
	
	if constexpr (isVector(temp))
	{
		int tabs;
		
		os << '{';
		
		for(const T& elem : vec)
		{
			os << '\n';
			
			tabs = tabCount+1;
			while (tabs--) os << '\t';
			
			printVector(os, elem, tabCount+1);
			
			os << ',';
		}
		
		os << '\n';
		
		tabs = tabCount;
		while (tabs--) os << '\t';
		
		os << '}';
	}
	else
	{
		os << '{';
		
		for(const T& elem : vec)
		{
			os << ' ' << elem << ',';
		}
		
		os << '}';
	}
}

inline std::ostream& operator<<(std::ostream& os, const Weight& obj)
{
	os << obj.m_value;
	
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const ModelParams& obj)
{
	os << "\n{" << 
	"\n\tm_noOfInputs = " << obj.m_noOfInputs << 
	"\n\tm_noOfLayers = " << obj.m_noOfLayers << 
	"\n\tm_learningRate = " << obj.m_learningRate << 
	"\n\tm_noOfNeuronsList = {";
	
	for(const unsigned int& neuronNo : obj.m_noOfNeuronsList) os << ' ' << neuronNo << ',';
	
	os << "}\n\tm_weightList = ";
	printVector(os, obj.m_weightList, 1);
	
	// for(const std::vector<std::vector<Weight>>& fromNeuronLayer : obj.m_weightList)
	// {
		// os << "\n\t\t{";
		
		// for(const std::vector<Weight>& toNeuronList : fromNeuronLayer)
		// {
			// os << "\n\t\t\t{";
			
			// for(const Weight& toNeuronNo : toNeuronList)
			// {
				// os << ' ' << toNeuronNo << ',';
			// }
			
			// os << "},";
		// }
		
		// os << "\n\t\t},";
	// }
	
	// os << "\n\t}\n\tm_biasList = {";
	
	os << "\n\tm_biasList = ";
	printVector(os, obj.m_biasList, 1);
	
	os << "\n}";
	
	return os;
}