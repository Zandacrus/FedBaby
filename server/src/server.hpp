#pragma once

#include <functional>
#include "model/include/ModelParams.hpp"


class Server
{
	public:
	
	Server(
		int submitPort, int requestPort, 
		unsigned long long serverModelParamsWeightage
	);
	
	void start(void);
	
	private:
	
	int m_submitPort;
	int m_requestPort;
	unsigned long long m_noOfClientSubmissions;
	ModelParams m_serverModelParams;
	
	bool createSocket(int& serverSocket);
	
	bool bindSocket(int& serverSocket, int& port);
	
	bool listenForConnections(int& serverSocket);
	
	void acceptConnections(
		int& serverSocket, std::function<void(const int&)> handler
	);
	
	void updateServerModelParams(const ModelParams& clientModelParams);
	
	bool receiveModelParamsFromClient(
		int p_clientSocket, ModelParams& p_recipientModelParams
	);
	
	bool sendModelParamsToClient(
		int p_clientSocket, const ModelParams& p_modelParamsToSend
	);
};
