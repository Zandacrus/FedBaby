# FedBaby
A very simple federated learning model based on Binary classification implemented in c++.
### Building - 
Linux environment is required to compile the server and client. Any linux distro or Windows Subsystem for Linux should work. <br>
<br>
First git clone the repository into your computer. <br>
To compile the server run the following command in the root folder of the repository - 
````
g++ -pthread -o server.out server/src/*.cpp -I"<complete path to the root folder of the repo>" -std=c++17
````

To compile the client run the following command in the root folder of the repository - 
````
g++ -pthread -o client.out model/src/*.cpp client/src/*.cpp -I"<complete path to the root folder of the repo>" -std=c++17
````

Then run the server in the root folder of the repository as follows - 
````
./server.out
````

Run the client in the root folder of the repository in the same computer as follows - 
````
./client.out
````
