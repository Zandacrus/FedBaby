#pragma once

#include "FeedForwardNeuralNetwork.hpp"
#include <vector>

class BinaryClassifier : public FeedForwardNeuralNetwork
{
	public:
	
	BinaryClassifier(
		unsigned int p_noOfInputs, 
		unsigned int p_noOfLayers, 
		std::vector<unsigned int> p_noOfNeuronsList, 
		long double p_initialBias = 1, 
		long double p_initialWeight = 1
	) : 
	FeedForwardNeuralNetwork(
		p_noOfInputs, 
		p_noOfLayers, 
		p_noOfNeuronsList, 
		p_initialBias, 
		p_initialWeight
	)
	{}
	
	long double train(
		std::vector<std::vector<long double>> p_inputs, 
		std::vector<std::vector<long double>> p_expectedOutputs, 
		long double p_learningRate = 0
	);
	
	long double train(
		std::vector<long double> p_input, 
		std::vector<long double> p_expectedOutput, 
		long double p_learningRate = 0
	);
	
	long double test(
		std::vector<std::vector<long double>> p_inputs, 
		std::vector<std::vector<long double>> p_expectedOutputs
	);
};
