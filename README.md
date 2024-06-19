# FedBaby
A very simple federated learning model based on Binary classification implemented in c++.
### Building - 
Linux environment is required to compile the server and client. Any linux distro or Windows Subsystem for Linux should work. <br>
<br>
To compile the server run the following command in the root folder of the repository - 
````
g++ -pthread -o server.out server/src/*.cpp -I"/mnt/d/Projects/Machine Learning/Federated Learning/FedBaby/" -std=c++17
````

To compile the client run the following command in the root folder of the repository - 
````
g++ -pthread -o client.out model/src/*.cpp client/src/*.cpp -I"/mnt/d/Projects/Machine Learning/Federated Learning/FedBaby/" -std=c++17
````

Then run the server as follows - 
````
./server.out
````

Run the client in the same computer - 
````
./client.out
````
