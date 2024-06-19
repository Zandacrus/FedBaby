#include "FeedForwardNeuralNetwork.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

// #define DEBUG


FeedForwardNeuralNetwork::FeedForwardNeuralNetwork(
	unsigned int p_noOfInputs, 
	unsigned int p_noOfLayers, 
	std::vector<unsigned int> p_noOfNeuronsList, 
	long double p_initialWeight, 
	long double p_initialBias
) : 
	m_modelParams(
		p_noOfInputs, 
		p_noOfLayers, 
		0.001, 
		p_noOfNeuronsList, 
		p_initialWeight,
		p_initialBias
	), 
	m_cost(0), 
	m_noOfTrainedExamples(0)
{
	m_network.push_back(std::vector<Neuron>(m_modelParams.m_noOfInputs));
	
    for(
		unsigned int layerNo = 1; 
		layerNo <= m_modelParams.m_noOfLayers; 
		layerNo++
	)
    {
		m_network.push_back(
			std::vector<Neuron>(m_modelParams.m_noOfNeuronsList[layerNo])
		);
		
		m_derivativeOfCostWrtBias.push_back(
			std::vector<long double>(m_modelParams.m_noOfNeuronsList[layerNo], 0)
		);
	}
	
	m_expectedOutput 
	= std::vector<long double>(m_modelParams.m_noOfNeuronsList[m_modelParams.m_noOfLayers], 0.0);
}

void FeedForwardNeuralNetwork::input(
	std::vector<long double> p_input, 
	std::vector<long double> p_expectedOutput, 
	long double p_learningRate
)
{
	assert(p_input.size() == m_modelParams.m_noOfInputs);
	
	for (unsigned int inputNo = 0; inputNo < m_modelParams.m_noOfInputs; inputNo++)
	{
		// activationFunction - sigmoid
		m_network[0][inputNo].m_activation = 1.0L/(
			1.0L + expl(-p_input[inputNo])
		);
	}
	
	assert(p_expectedOutput.size() == m_expectedOutput.size());
	
	m_expectedOutput = p_expectedOutput;
	
	m_modelParams.m_learningRate = p_learningRate;
	
	m_noOfTrainedExamples++;
}

std::vector<Neuron> FeedForwardNeuralNetwork::output(void)
{
	return m_network[m_modelParams.m_noOfLayers];
}

void FeedForwardNeuralNetwork::forwardPropagate(void)
{
	for (
		unsigned int layerNo = 1; 
		layerNo <= m_modelParams.m_noOfLayers; 
		layerNo++
	)
	{
		for(
			unsigned int neuronNo = 0; 
			neuronNo < m_modelParams.m_noOfNeuronsList[layerNo]; 
			neuronNo++
		)
		{
			m_network[layerNo][neuronNo].m_activation
			= m_modelParams.m_biasList[layerNo-1][neuronNo];
			// = m_network[layerNo][neuronNo].m_bias;
			
			for(
				unsigned int fromNeuronNo = 0; 
				fromNeuronNo < m_network[layerNo-1].size(); 
				fromNeuronNo++
			)
			{
				m_network[layerNo][neuronNo].m_activation
				+= (
					m_modelParams.m_weightList[layerNo-1][fromNeuronNo][neuronNo].m_value 
					* m_network[layerNo-1][fromNeuronNo].m_activation
				);
			}
			
			// activationFunction - sigmoid
			m_network[layerNo][neuronNo].m_activation = 1.0L/(
				1.0L + expl(-m_network[layerNo][neuronNo].m_activation)
			);
		}
	}
}

// Vars representing no of neurons in respective ...
// ... layers	- a, b, ..., h, i, j
// Ej			- jth expected output 
// CostFunc 	- J(Aj) 		= SUM(trainedSamples){SUM(j){(Aj-Ej)**2}}/(2*trainedSamplesNo)
// Activation 	- Aj(Zj) 		= SIGMOID(Zj)
// Zj 			- Zj(Wji,Ai,Bj)	= SUM(i){Wji*Ai}+Bj
// 

// dJ/dBj	= (dZj/dBj)*(dAj/dZj)*(dJ/dAj)
// dJ/dBj	= 1*(dAj/dZj)*(dJ/dAj)
// dJ/dBj	= (dAj/dZj)*2*(Aj-Ej)

// dJ/dWji	=(dZj/dWji)*(dAj/dZj)*(dJ/dAj)
// dJ/dWji	= Ai*(dJ/Bj)

// dJ/dBi	= SUM(j){(dZi/dBi)*(dAi/dZi)*(dZj/dAi)*(dAj/dZj)*(dJ/dAj)}
// dJ/dBi	= (dZi/dBi)*(dAi/dZi)*SUM(j){Wji*(dJ/dBj)}
// dJ/dBi	= 1*(dAi/dZi)*SUM(j){Wji*(dJ/dBj)}

// dJ/dWih	= (dZi/dWih)*(dJ/dZi)
// dJ/dWih	= Ah*(dJ/dBi) 			[since, (dJ/dBi) = (dJ/dZi)*(dZi/dBi) = (dJ/dZi)*1]

// dJ/dBh = (dZh/dBh)*(dAh/dZh)*(dJ/dAh)
// dJ/dBh = 1*(dAh/dZh)*SUM(i){(dZi/dAh)*(dJ/dZi)}
// dJ/dBh = (dAh/dZh)*SUM(i){Wih*(dJ/dBi)}			[since, (dJ/dBi) = (dJ/dZi)]

// dJ/dWhg = (dZh/dWhg)*(dJ/dZh)
// dJ/dWhg = Ag*(dJ/dBh)			[since, (dJ/dBh) = (dJ/dZh)]

void FeedForwardNeuralNetwork::backPropagate(void) // gradient descent
{
	int layerNo, neuronNo, fromNeuronNo, toNeuronNo;
	
	#ifdef DEBUG
	std::cout << "start\n";
	#endif
	
	m_cost *= 2 * (m_noOfTrainedExamples - 1);
	
	for(
		layerNo = m_modelParams.m_noOfLayers, neuronNo = 0;
		neuronNo < m_modelParams.m_noOfNeuronsList[layerNo];
		neuronNo++
	)
	{
		m_cost += (
			m_network[layerNo][neuronNo].m_activation
			-m_expectedOutput[neuronNo]
		)*(
			m_network[layerNo][neuronNo].m_activation
			-m_expectedOutput[neuronNo]
		);
		
		// dAj/dZj = Aj*(1-Aj)
		// dJ/dBj	= (dAj/dZj)*2*(Aj-Ej)
		
		m_derivativeOfCostWrtBias[layerNo-1][neuronNo]
		= (
			m_network[layerNo][neuronNo].m_activation
			*(1-m_network[layerNo][neuronNo].m_activation)
			*2*(
				m_network[layerNo][neuronNo].m_activation
				-m_expectedOutput[neuronNo]
			)
		);
		
		#ifdef DEBUG
		std::cout << "\nm_network[" << layerNo << "][" << neuronNo 
		<< "].m_activation = " 
		<< m_network[layerNo][neuronNo].m_activation
		<< '\n';
		
		std::cout << "\nm_expectedOutput[" << neuronNo << "] = " 
		<< m_expectedOutput[neuronNo] << '\n';
		#endif
		
		// new{Bj} = old{Bj} - LR * (dJ/Bj)
		
		// m_network[layerNo][neuronNo].m_bias
		m_modelParams.m_biasList[layerNo-1][neuronNo]
		-= m_modelParams.m_learningRate
		*m_derivativeOfCostWrtBias[layerNo-1][neuronNo];
	}
	
	m_cost /= 2 * m_noOfTrainedExamples;
	
	for(
		layerNo = m_modelParams.m_noOfLayers-1;
		layerNo >= 1;
		layerNo--
	)
	{
		for(
			fromNeuronNo = 0;
			fromNeuronNo < m_modelParams.m_noOfNeuronsList[layerNo];
			fromNeuronNo++
		)
		{
			// dA[x]/dZ[x] = A[x]*(1-A[x])
			// dJ/dB[x] = (dA[x]/dZ[x])*SUM([x+1]){W[x+1][x]*(dJ/dB[x+1])}
			
			m_derivativeOfCostWrtBias[layerNo-1][fromNeuronNo] = 0;
			
			for(
				toNeuronNo = 0;
				toNeuronNo < m_modelParams.m_noOfNeuronsList[layerNo+1];
				toNeuronNo++
			)
			{
				m_derivativeOfCostWrtBias[layerNo-1][fromNeuronNo]
				+= (
					m_modelParams.m_weightList[layerNo][fromNeuronNo][toNeuronNo].m_value
					*m_derivativeOfCostWrtBias[layerNo][toNeuronNo]
				);
				
				#ifdef DEBUG
				std::cout << "m_modelParams.m_weightList[" << layerNo << "][" << fromNeuronNo << "][" << toNeuronNo << "].m_value = " 
				<< m_modelParams.m_weightList[layerNo][fromNeuronNo][toNeuronNo].m_value << '\n';
				
				std::cout << "m_derivativeOfCostWrtBias[" << layerNo << "][" << toNeuronNo << "] = " 
				<< m_derivativeOfCostWrtBias[layerNo][toNeuronNo] << '\n';
				#endif
			}
			
			m_derivativeOfCostWrtBias[layerNo-1][fromNeuronNo]
			*= m_network[layerNo][fromNeuronNo].m_activation
			*(1-m_network[layerNo][fromNeuronNo].m_activation);
		}
	}
	
	for(
		layerNo = m_modelParams.m_noOfLayers-1;
		layerNo >= 0;
		layerNo--
	)
	{
		for(
			fromNeuronNo = 0;
			fromNeuronNo < m_modelParams.m_noOfNeuronsList[layerNo];
			fromNeuronNo++
		)
		{
			for(
				toNeuronNo = 0;
				toNeuronNo < m_modelParams.m_noOfNeuronsList[layerNo+1];
				toNeuronNo++
			)
			{
				// dJ/dW[x+1][x] = A[x]*(dJ/dB[x+1])
				// new{W[x+1][x]} = old{W[x+1][x]} - LR * (dJ/dW[x+1][x])
				
				m_modelParams.m_weightList[layerNo][fromNeuronNo][toNeuronNo].m_value
				-= m_modelParams.m_learningRate*(
					m_network[layerNo][fromNeuronNo].m_activation
					*m_derivativeOfCostWrtBias[layerNo][toNeuronNo]
				);
				
				#ifdef DEBUG
				std::cout << "\nm_network[" << layerNo << "][" << fromNeuronNo 
				<< "].m_activation = " 
				<< m_network[layerNo][fromNeuronNo].m_activation
				<< '\n';
				
				std::cout << "m_derivativeOfCostWrtBias[" << layerNo << "][" << toNeuronNo 
				<< "] = " 
				<< m_derivativeOfCostWrtBias[layerNo][toNeuronNo]
				<< '\n';
				
				std::cout << "m_weightList[" << layerNo << "][" << fromNeuronNo 
				<< "][" << toNeuronNo << "].m_value = " 
				<< m_modelParams.m_weightList[layerNo][fromNeuronNo][toNeuronNo].m_value
				<< '\n';
				#endif
			}
			
			if (layerNo != 0)
			{
				// new{B[x]} = old{B[x]} - LR * (dJ/B[x])
				
				// m_network[layerNo][fromNeuronNo].m_bias
				m_modelParams.m_biasList[layerNo-1][fromNeuronNo]
				-= m_modelParams.m_learningRate
				*m_derivativeOfCostWrtBias[layerNo-1][fromNeuronNo];
			}
		}
	}
	
	#ifdef DEBUG
	std::cout << "end\n";
	#endif
}

long double FeedForwardNeuralNetwork::cost(void)
{
	return m_cost;
}

void FeedForwardNeuralNetwork::print(void)
{
	std::cout << "\nm_modelParams = " << m_modelParams << '\n';
	
	std::cout << "m_cost = " << m_cost << '\n';
	std::cout << "m_noOfTrainedExamples = " << m_noOfTrainedExamples << '\n';
	
	std::cout << "\nm_expectedOutput = \n";
	for (auto& x : m_expectedOutput) std::cout << x << ' ';
	
	std::cout << '\n';
	
	// m_network, m_derivativeOfCostWrtBias
}
