#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include "server.hpp"
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>



static volatile std::sig_atomic_t sigintSignal = 0;

void sigintHandler(int signal)
{
	sigintSignal = 1;
}


Server::Server(
	int submitPort, int requestPort, 
	unsigned long long serverModelParamsWeightage
) : 
	m_submitPort(submitPort), m_requestPort(requestPort), 
	m_noOfClientSubmissions(serverModelParamsWeightage), 
	m_serverModelParams(11, 1, 0.001, {1}, 1)
{}

void Server::start(void)
{
	std::thread submitThread(
		[this] ()
		{
			int submitSocket;
			
			if (createSocket(submitSocket))
			{
				if(
					sigintSignal == 0
					&& bindSocket(submitSocket, m_submitPort) 
					&& listenForConnections(submitSocket)
				) {
					acceptConnections(
						submitSocket, 
						[this] (const int& clientSocket)
						{
							ModelParams receivedModelParams;
							
							if (receiveModelParamsFromClient(clientSocket, receivedModelParams))
							{
								updateServerModelParams(receivedModelParams);
							}
							
							close(clientSocket);
						}
					);
				}
				
				close(submitSocket);
			}
		}
	);
	
	std::thread requestThread(
		[this] ()
		{
			int requestSocket;
			
			if (createSocket(requestSocket))
			{
				if (
					sigintSignal == 0
					&& bindSocket(requestSocket, m_requestPort) 
					&& listenForConnections(requestSocket)
				) {
					acceptConnections(
						requestSocket, 
						[this] (const int& clientSocket)
						{
							sendModelParamsToClient(clientSocket, m_serverModelParams);
							close(clientSocket);
						}
					);
				}
				
				close(requestSocket);
			}
		}
	);
	
	submitThread.join();
	requestThread.join();
}

bool Server::createSocket(int& serverSocket)
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (serverSocket == -1)
	{
		std::cerr << "Error creating socket\n";
		return false;
	}
	
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Error setting socket options\n";
		
		close(serverSocket);
		return false;
	}
	
	int flags = fcntl(serverSocket, F_GETFL, 0);
	fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);
	
	return true;
}

bool Server::bindSocket(int& serverSocket, int& port)
{
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
	
	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Error binding socket, port = " << port << ".\n";
		
		close(serverSocket);
		return false;
	}
	
	std::cout << "Binded socket = " << serverSocket << ", port = " << port << ".\n";
	
	return true;
}

bool Server::listenForConnections(int& serverSocket)
{
	if (listen(serverSocket, 10) == -1)
	{
		std::cerr << "Error listening on socket = " << serverSocket << ".\n";
		
		close(serverSocket);
		return false;
	}
	
	return true;
}

void Server::acceptConnections(
	int& serverSocket, std::function<void(const int&)> handler
)
{
	while (sigintSignal == 0)
	{
		struct sockaddr_in clientAddr;
		socklen_t clientLen = sizeof(clientAddr);
		int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
		
		if (clientSocket == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
			
			std::cerr << "Error accepting connection, serverSocket = " << serverSocket << "\n";
			continue;
		}
		
		handler(clientSocket);
	}
}

void Server::updateServerModelParams(const ModelParams& p_clientModelParams)
{
	std::cout << "New ModelParams = " << p_clientModelParams << '\n';
	
	for(unsigned int layerNo=0; layerNo<m_serverModelParams.m_noOfLayers; layerNo++)
	{
		for(unsigned int fromNeuronNo=0; fromNeuronNo<m_serverModelParams.m_noOfNeuronsList[layerNo]; fromNeuronNo++)
		{
			for(unsigned int toNeuronNo=0; toNeuronNo<m_serverModelParams.m_noOfNeuronsList[layerNo+1]; toNeuronNo++)
			{
				m_serverModelParams.m_weightList[layerNo][fromNeuronNo][toNeuronNo].m_value = (
					m_serverModelParams.m_weightList[layerNo][fromNeuronNo][toNeuronNo].m_value
					*m_noOfClientSubmissions
					+p_clientModelParams.m_weightList[layerNo][fromNeuronNo][toNeuronNo].m_value
				)/(m_noOfClientSubmissions+1);
			}
		}
	}
	
	std::cout << "Updated ModelParams = " << m_serverModelParams << '\n';
}

bool Server::receiveModelParamsFromClient(
	int p_clientSocket, ModelParams& p_recipientModelParams
)
{
	int bytesReceived;
	
	p_recipientModelParams = m_serverModelParams;
	
	for(unsigned int layerNo=0; layerNo<p_recipientModelParams.m_noOfLayers; layerNo++)
	{
		for(unsigned int fromNeuronNo=0; fromNeuronNo<p_recipientModelParams.m_noOfNeuronsList[layerNo]; fromNeuronNo++)
		{
			bytesReceived = recv(
				p_clientSocket, 
				p_recipientModelParams.m_weightList[layerNo][fromNeuronNo].data(), 
				sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_recipientModelParams.m_weightList[layerNo][fromNeuronNo])>>::value_type)
				*p_recipientModelParams.m_weightList[layerNo][fromNeuronNo].size(), 
				0
			);
			
			if (bytesReceived == -1) {
				std::cerr << "Error receiving ModelParams.m_weightList[" << layerNo << "][" << fromNeuronNo << "] from client\n";
				return false;
			}
		}
	}
	
	for(unsigned int layerNo=0; layerNo<p_recipientModelParams.m_noOfLayers; layerNo++)
	{
		bytesReceived = recv(
			p_clientSocket,
			p_recipientModelParams.m_biasList[layerNo].data(), 
			sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_recipientModelParams.m_biasList[layerNo])>>::value_type)
			*p_recipientModelParams.m_biasList[layerNo].size(), 
			0
		);
		
		if (bytesReceived == -1) {
			std::cerr << "Error receiving ModelParams.m_biasList[" << layerNo << "] from client\n";
			return false;
		}
	}
	
	return true;
}

bool Server::sendModelParamsToClient(
	int p_clientSocket, const ModelParams& p_modelParamsToSend
)
{
	int bytesSent = send(p_clientSocket, &p_modelParamsToSend.m_noOfInputs, sizeof(p_modelParamsToSend.m_noOfInputs), 0);
	
	if (bytesSent == -1)
	{
		std::cerr << "Error sending ModelParams.m_noOfInputs to client\n";
		return false;
	}
	
	bytesSent = send(p_clientSocket, &p_modelParamsToSend.m_noOfLayers, sizeof(p_modelParamsToSend.m_noOfLayers), 0);
	
	if (bytesSent == -1)
	{
		std::cerr << "Error sending ModelParams.m_noOfLayers to client\n";
		return false;
	}
	
	bytesSent = send(p_clientSocket, &p_modelParamsToSend.m_learningRate, sizeof(p_modelParamsToSend.m_learningRate), 0);
	
	if (bytesSent == -1)
	{
		std::cerr << "Error sending ModelParams.m_learningRate to client\n";
		return false;
	}
	
	bytesSent = send(
		p_clientSocket, 
		p_modelParamsToSend.m_noOfNeuronsList.data(), 
		sizeof(std::remove_cv_t<std::remove_reference_t<decltype(p_modelParamsToSend.m_noOfNeuronsList)>>::value_type)
		*p_modelParamsToSend.m_noOfNeuronsList.size(), 
		0
	);
	
	if (bytesSent == -1)
	{
		std::cerr << "Error sending ModelParams.m_noOfNeuronsList to client\n";
		return false;
	}
	
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
				std::cerr << "Error sending ModelParams.m_weightList[" << layerNo << "][" << fromNeuronNo << "] to client\n";
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
			std::cerr << "Error sending ModelParams.m_biasList[" << layerNo << "] to client\n";
			return false;
		}
	}
	
	return true;
}

int main()
{
	std::signal(SIGINT, sigintHandler);
	Server server(12347, 12348, 1);
	server.start();
}
