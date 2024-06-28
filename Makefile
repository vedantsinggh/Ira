SRC := 
INCLUDE := 
ARGS := -Wall -Wextra

build: ira.cpp $(SRC) $(INCLUDE)
	g++ -I$(INCLUDE) $(SRC) $(ARGS) ira.cpp -o ira

run: app.ira build 
	./ira app.ira

