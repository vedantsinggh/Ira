SRC := 
INCLUDE := 
ARGS := -Wall -Wextra

build: ira.cpp $(SRC) $(INCLUDE)
	g++ -I$(INCLUDE) $(SRC) $(ARGS) ira.cpp -o ira

debug-build: ira.cpp $(SRC) $(INCLUDE)
	g++ -D debug -I$(INCLUDE) $(SRC) $(ARGS) ira.cpp -o ira

run: app.ira build 
	./ira app.ira

debug: app.ira debug-build
	./ira app.ira

