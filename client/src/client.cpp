#include <algorithm>
#include <atomic>
#include <arpa/inet.h>
#include <chrono>
#include "client.hpp"
#include <cstring>
#include "data/heart_disease/reader.hpp"
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <random>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>



Client::Client(
	const std::string& p_serverIP, int p_submitPort, 
	int p_requestPort
) : 
	Model(), 
	m_serverIP(p_serverIP), m_submitPort(p_submitPort), 
	m_requestPort(p_requestPort)
{}

bool Client::submitModelParams()
{
	int clientSocket = createSocket();
	if (clientSocket == -1) return false;
	
	if (connectToServer(clientSocket, m_submitPort))
	{
		std::cout << "Submitting ModelParams = " << Model::m_modelParams << '\n';
		if (sendModelParamsToServer(clientSocket, Model::m_modelParams))
		{
			close(clientSocket);
			return true;
		}
	}
	
	close(clientSocket);
	return false;
}

bool Client::requestModelParams()
{
	int clientSocket = createSocket();
	if (clientSocket == -1) return false;
	
	if (connectToServer(clientSocket, m_requestPort))
	{
		ModelParams receivedModelParams;
		
		if (receiveModelParamsFromServer(clientSocket, receivedModelParams))
		{
			std::cout << "Received ModelParams = " << receivedModelParams << '\n';
			
			Model::updateModelParams(receivedModelParams);
			std::cout << "Updated ModelParams = " << Model::m_modelParams << '\n';
			
			close(clientSocket);
			return true;
		}
	}
	
	close(clientSocket);
	return false;
}

int Client::createSocket()
{
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (clientSocket == -1)
	{
		std::cerr << "Error creating socket\n";
		return -1;
	}
	
	return clientSocket;
}

bool Client::connectToServer(int p_clientSocket, int p_port)
{
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(p_port);
	inet_pton(AF_INET, m_serverIP.c_str(), &serverAddr.sin_addr);
	
	if (connect(p_clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Error connecting to server, port = " << p_port << ".\n";
		return false;
	}
	
	return true;
}

bool Client::sendModelParamsToServer(
	int p_clientSocket, const ModelParams& p_modelParamsToSend
)
{
	int bytesSent;
	
	for(unsigned int layerNo=0; layerNo<p_modelParamsToSend.m_noOfLayers; layerNo++)
	{
		for(unsigned int fromNeuronNo=0; fromNeuronNo<p_modelParamsToSend.m_noOfNeuronsList[layerNo]; fromNeuronNo++)
		{
			bytesSent = send(
				p_clientSocket, 
				p_modelParamsToSend.m_weightList[layerNo][fromNeuronNo].data(), 
				sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_modelParamsToSend.m_weightList[layerNo][fromNeuronNo])>>::value_type)
				*p_modelParamsToSend.m_weightList[layerNo][fromNeuronNo].size(), 
				0
			);
			
			if (bytesSent == -1) {
				std::cerr << "Error sending ModelParams.m_weightList[" << layerNo << "][" << fromNeuronNo << "] to server\n";
				return false;
			}
		}
	}
	
	for(unsigned int layerNo=0; layerNo<p_modelParamsToSend.m_noOfLayers; layerNo++)
	{
		bytesSent = send(
			p_clientSocket,
			p_modelParamsToSend.m_biasList[layerNo].data(), 
			sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_modelParamsToSend.m_biasList[layerNo])>>::value_type)
			*p_modelParamsToSend.m_biasList[layerNo].size(), 
			0
		);
		
		if (bytesSent == -1) {
			std::cerr << "Error sending ModelParams.m_biasList[" << layerNo << "] to server\n";
			return false;
		}
	}
	
	return true;
}

bool Client::receiveModelParamsFromServer(
	int p_clientSocket, ModelParams& p_recipientModelParams
)
{
	int bytesReceived = recv(p_clientSocket, &p_recipientModelParams.m_noOfInputs, sizeof(p_recipientModelParams.m_noOfInputs), 0);
	
	if (bytesReceived == -1) {
		std::cerr << "Error receiving ModelParams.m_noOfInputs from server\n";
		return false;
	}
	
	bytesReceived = recv(p_clientSocket, &p_recipientModelParams.m_noOfLayers, sizeof(p_recipientModelParams.m_noOfLayers), 0);
	
	if (bytesReceived == -1) {
		std::cerr << "Error receiving ModelParams.m_noOfLayers from server\n";
		return false;
	}
	
	bytesReceived = recv(p_clientSocket, &p_recipientModelParams.m_learningRate, sizeof(p_recipientModelParams.m_learningRate), 0);
	
	if (bytesReceived == -1) {
		std::cerr << "Error receiving ModelParams.m_learningRate from server\n";
		return false;
	}
	
	p_recipientModelParams.m_noOfNeuronsList.resize(
		p_recipientModelParams.m_noOfLayers+1
	);
	p_recipientModelParams.m_noOfNeuronsList.shrink_to_fit();
	
	bytesReceived = recv(
		p_clientSocket, 
		p_recipientModelParams.m_noOfNeuronsList.data(), 
		sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_recipientModelParams.m_noOfNeuronsList)>>::value_type)
		*p_recipientModelParams.m_noOfNeuronsList.size(), 
		0
	);
	
	if (bytesReceived == -1) {
		std::cerr << "Error receiving ModelParams.m_noOfNeuronsList from server\n";
		return false;
	}
	
	p_recipientModelParams.m_weightList.resize(
		p_recipientModelParams.m_noOfLayers
	);
	p_recipientModelParams.m_weightList.shrink_to_fit();
	
	for(unsigned int layerNo=0; layerNo<p_recipientModelParams.m_noOfLayers; layerNo++)
	{
		p_recipientModelParams.m_weightList[layerNo].resize(
			p_recipientModelParams.m_noOfNeuronsList[layerNo]
		);
		p_recipientModelParams.m_weightList[layerNo].shrink_to_fit();
		
		for(unsigned int fromNeuronNo=0; fromNeuronNo<p_recipientModelParams.m_noOfNeuronsList[layerNo]; fromNeuronNo++)
		{
			p_recipientModelParams.m_weightList[layerNo][fromNeuronNo].resize(
				p_recipientModelParams.m_noOfNeuronsList[layerNo+1]
			);
			p_recipientModelParams.m_weightList[layerNo][fromNeuronNo].shrink_to_fit();
			
			bytesReceived = recv(
				p_clientSocket, 
				p_recipientModelParams.m_weightList[layerNo][fromNeuronNo].data(), 
				sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_recipientModelParams.m_weightList[layerNo][fromNeuronNo])>>::value_type)
				*p_recipientModelParams.m_noOfNeuronsList[layerNo+1], 
				0
			);
			
			if (bytesReceived == -1) {
				std::cerr << "Error receiving ModelParams.m_weightList[" << layerNo << "][" << fromNeuronNo << "] from server\n";
				return false;
			}
		}
	}
	
	p_recipientModelParams.m_biasList.resize(
		p_recipientModelParams.m_noOfLayers
	);
	p_recipientModelParams.m_biasList.shrink_to_fit();
	
	for(unsigned int layerNo=0; layerNo<p_recipientModelParams.m_noOfLayers; layerNo++)
	{
		p_recipientModelParams.m_biasList[layerNo].resize(
			p_recipientModelParams.m_noOfNeuronsList[layerNo+1]
		);
		p_recipientModelParams.m_biasList[layerNo].shrink_to_fit();
		
		bytesReceived = send(
			p_clientSocket,
			p_recipientModelParams.m_biasList[layerNo].data(), 
			sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_recipientModelParams.m_biasList[layerNo])>>::value_type)
			*p_recipientModelParams.m_biasList[layerNo].size(), 
			0
		);
		
		if (bytesReceived == -1) {
			std::cerr << "Error receiving ModelParams.m_biasList[" << layerNo << "] from server\n";
			return false;
		}
	}
	
	return true;
}


enum class TaskState
{
	IDLE, SUBMITTING, REQUESTING, TRAINING, COMPLETED
};


int main()
{
	std::cout << "Reading data ...\n";
	
	std::map<std::string, std::vector<std::vector<long double>>> data;
	
	data = readData();
	
	std::cout << "Finished reading data.\n";
	
	std::random_device randDev;
	std::default_random_engine randGen1 {randDev()};
	std::default_random_engine randGen2 = randGen1;
	std::shuffle(data["inputs"].begin(), data["inputs"].end(), randGen1);
	std::shuffle(data["outputs"].begin(), data["outputs"].end(), randGen2);
	
	Client client("127.0.0.1", 12347, 12348);
	
	client.requestModelParams();
	
	// std::cout << client.getModelParams() << '\n';
	
	std::atomic<TaskState> taskState {TaskState::IDLE};
	
	std::thread submitThread(
		[&taskState, &client] ()
		{
			std::this_thread::sleep_for(std::chrono::minutes(5));
			TaskState expected;
			
			while (taskState.load() != TaskState::COMPLETED)
			{
				expected = TaskState::IDLE;
				if (
					taskState.compare_exchange_strong(
						expected, TaskState::SUBMITTING
					)
				)
				{
					client.submitModelParams();
					
					taskState.store(TaskState::IDLE);
					std::this_thread::sleep_for(std::chrono::minutes(5));
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
		}
	);
	
	std::thread requestThread(
		[&taskState, &client] ()
		{
			std::this_thread::sleep_for(std::chrono::minutes(7));
			
			while (taskState.load() != TaskState::COMPLETED)
			{
				TaskState expected {TaskState::IDLE};
				if (
					taskState.compare_exchange_strong(
						expected, TaskState::REQUESTING
					)
				)
				{
					client.requestModelParams();
					
					taskState.store(TaskState::IDLE);
					std::this_thread::sleep_for(std::chrono::minutes(5));
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
		}
	);
	
	std::thread trainThread(
		[&taskState, &client, &data] ()
		{
			unsigned int trainingSampleNo = 0;
			TaskState expected;
			
			while (trainingSampleNo < data["inputs"].size())
			{
				expected = TaskState::IDLE;
				if (
					taskState.compare_exchange_strong(
						expected, TaskState::TRAINING
					)
				)
				{
					client.train(
						data["inputs"][trainingSampleNo], 
						data["outputs"][trainingSampleNo]
					);
					
					trainingSampleNo++;
					
					if (trainingSampleNo < data["inputs"].size())
					{
						taskState.store(TaskState::IDLE);
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
					else taskState.store(TaskState::COMPLETED);
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
		}
	);
	
	trainThread.join();
	submitThread.join();
	requestThread.join();
	
	client.submitModelParams();
	client.requestModelParams();
	
	std::cout << "Testing model...\n Accuracy = " 
	<< client.test(data["inputs"], data["outputs"]) << '\n';
	
	return 0;
}
