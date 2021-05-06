CXX		= g++
CXXFLAGS= -g -Wall
EXE_SERV= server
EXE_SUBS= subscriber

# Server port
PORT = 12345

# Server IP address
IP_SERVER = 127.0.0.1

USER = George

all: clean $(EXE_SERV) 

# Compile Server.cpp
server: ServerClass.cpp server.cpp
	$(CXX) $(CXXFLAGS) Topic.cpp ServerClass.cpp ClientClass.cpp server.cpp -o $(EXE_SERV)

# Compile Server.cpp
subscriber: ClientClass.cpp subscriber.cpp
	$(CXX) $(CXXFLAGS) Topic.cpp ClientClass.cpp subscriber.cpp -o $(EXE_SUBS)

.PHONY: clean run_server 

run_server:
	./$(EXE_SERV) ${PORT}

run_subscriber:
	./$(EXE_SUBS) ${USER} ${IP_SERVER} ${PORT}

clean:
	rm -f ./${EXE_SERV}
	rm -f ./${EXE_SUBS}
	rm -f ./*.o 
