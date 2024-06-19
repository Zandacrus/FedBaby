#pragma once

#include "model/src/BinaryClassifier.hpp"


class Model : public BinaryClassifier
{
	public:
	
	Model() : 
	BinaryClassifier(
		0, 0, {}
	)
	{}
	
	void updateModelParams(const ModelParams& newModelParams)
	{
		m_modelParams = newModelParams;
		
		m_network.resize(
			m_modelParams.m_noOfLayers+1
		);
		m_derivativeOfCostWrtBias.resize(
			m_modelParams.m_noOfLayers
		);
		
		m_network[0].resize(
			m_modelParams.m_noOfInputs
		);
		
		for(
			unsigned int layerNo = 1; 
			layerNo <= m_modelParams.m_noOfLayers; 
			layerNo++
		)
		{
			m_network[layerNo].resize(
				m_modelParams.m_noOfNeuronsList[layerNo]
			);
			
			m_derivativeOfCostWrtBias[layerNo-1].resize(
				m_modelParams.m_noOfNeuronsList[layerNo]
			);
		}
		
		m_expectedOutput.resize(
			m_modelParams.m_noOfNeuronsList[m_modelParams.m_noOfLayers]
		);
	}
	
	protected:
	
	using FeedForwardNeuralNetwork::m_modelParams;
};
