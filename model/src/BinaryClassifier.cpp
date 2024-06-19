#include "BinaryClassifier.hpp"
#include <cassert>
// #include <iostream>


long double BinaryClassifier::train(
	std::vector<std::vector<long double>> p_inputs, 
	std::vector<std::vector<long double>> p_expectedOutputs, 
	long double p_learningRate
)
{
	assert(p_inputs.size() == p_expectedOutputs.size());
	
	if (p_learningRate == 0)
	{
		p_learningRate = FeedForwardNeuralNetwork::m_modelParams.m_learningRate;
	}
	
	for(unsigned int i = 0; i < p_inputs.size(); i++)
	{
		FeedForwardNeuralNetwork::input(p_inputs[i], p_expectedOutputs[i], p_learningRate);
		FeedForwardNeuralNetwork::forwardPropagate();
		FeedForwardNeuralNetwork::backPropagate();
		// FeedForwardNeuralNetwork::print();
	}
	
	return FeedForwardNeuralNetwork::cost();
}

long double BinaryClassifier::train(
	std::vector<long double> p_input, 
	std::vector<long double> p_expectedOutput, 
	long double p_learningRate
)
{
	if (p_learningRate == 0)
	{
		p_learningRate = FeedForwardNeuralNetwork::m_modelParams.m_learningRate;
	}
	
	FeedForwardNeuralNetwork::input(p_input, p_expectedOutput, p_learningRate);
	FeedForwardNeuralNetwork::forwardPropagate();
	FeedForwardNeuralNetwork::backPropagate();
	// FeedForwardNeuralNetwork::print();
	
	return FeedForwardNeuralNetwork::cost();
}

long double BinaryClassifier::test(
	std::vector<std::vector<long double>> p_inputs, 
	std::vector<std::vector<long double>> p_expectedOutputs
)
{
	assert(p_inputs.size() == p_expectedOutputs.size());
	
	unsigned int success = 0;
	
	for(unsigned int sampleNo = 0; sampleNo < p_inputs.size(); sampleNo++)
	{
		FeedForwardNeuralNetwork::input(
			p_inputs[sampleNo], 
			p_expectedOutputs[sampleNo], 
			FeedForwardNeuralNetwork::m_modelParams.m_learningRate
		);
		
		FeedForwardNeuralNetwork::forwardPropagate();
		
		if (
			static_cast<int>(FeedForwardNeuralNetwork::output()[0].m_activation+0.5L) 
			== p_expectedOutputs[sampleNo][0]
		)
		{
			success++;
		}
		
		// std::cout << "accuracy = " << static_cast<long double>(success)/(sampleNo+1) << '\n';
	}
	
	return static_cast<long double>(success)/p_inputs.size();
}
