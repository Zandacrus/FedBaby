#pragma once

#include "model/include/ModelParams.hpp"
#include <vector>


struct Neuron
{
	long double m_activation = 1;
};

class FeedForwardNeuralNetwork
{
	public:
	
	FeedForwardNeuralNetwork(
		unsigned int p_noOfInputs, 
		unsigned int p_noOfLayers, 
		std::vector<unsigned int> p_noOfNeuronsList, 
		long double p_initialBias = 0, 
		long double p_initialWeight = 1
	);
	
	void input(
		std::vector<long double> p_input, 
		std::vector<long double> p_expectedOutput, 
		long double p_learningRate = 0.001
	);
	
	std::vector<Neuron> output(void);
	
	void forwardPropagate(void);
	
	void backPropagate(void);
	
	long double cost(void);
	
	void print(void);
	
	protected:
	
	ModelParams m_modelParams;
	long double m_cost;
	double m_noOfTrainedExamples;
    std::vector<std::vector<Neuron>> m_network;
    std::vector<std::vector<long double>> m_derivativeOfCostWrtBias;
    std::vector<long double> m_expectedOutput;
	
	// void optimizer(layerNo, fromNeuronNo, toNeuronNo);
	
	// void aggregationFunction(
		// unsigned int p_layerNo, 
		// unsigned int p_neuronNo
	// );
	
	// long double activationFunction(long double);
};
