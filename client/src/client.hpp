#pragma once

#include "model/include/Model.hpp"
#include "model/include/ModelParams.hpp"
#include <string>


class Client : public Model
{
	public:
	
	Client(
		const std::string& p_serverIP, int p_submitPort, 
		int p_requestPort
	);
	
	bool submitModelParams();
	
	bool requestModelParams();
	
	const ModelParams& getModelParams() const
	{
		return Model::m_modelParams;
	}
	
	private:
	
	std::string m_serverIP;
	int m_submitPort; // port to submit model params to server 
	int m_requestPort; // port to request model params from server 
	
	int createSocket();
	
	bool connectToServer(int p_clientSocket, int p_port);
	
	bool sendModelParamsToServer(
		int p_clientSocket, const ModelParams& p_modelParamsToSend
	); // data serializing funtion while sending model params to server
	
	bool receiveModelParamsFromServer(
		int p_clientSocket, ModelParams& p_recipientModelParams
	); // data deserializing funtion while receiving model params from server
};
