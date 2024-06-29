#include <iostream>
using std::cout;
using std::endl;

#include <iomanip>
using std::setw;

#include <fstream>

#include <string>
using std::string;

#include <vector>
using std::vector;

#define printp(x)   std::cout<<x
#define println(x)  std::cout<<x<<std::endl
#define printerr(x) std::cerr<<x<<std::endl

enum TokenType {
	INTEGER,
	FLOAT,
	STRING,
	IDENTIFIER,
	OPERATOR,
	KEYWORD,
};

enum OperatorCode {
	ASSIGN,
};

enum KeywordCode {
	PRINT,
};

struct Token {
	TokenType type;

	struct Position {
		short row, col;
	};

	Position position;
	string value;
};

struct Float {
	string name;
	float value;
};

struct Integer {
	string name;
	int value;
};

struct String {
	string name;
	string value;
};

struct Chunk {
	TokenType type;
	string value;
};

struct Context {
	vector<Float>    floats;
	vector<Integer>  integers;
	vector<String>   strings;
	vector<Chunk>    memory;
};

bool isValidNumber(std::string& str)
{
	for (char ch : str) {
	    int v = ch;
	    if (!(ch >= 48 && ch <= 57)) {
		return false;
	    }
	}
	return true; 
}

vector<string> identifiers;
bool shouldBeIdentifier = false;
Token tokenizeWord(string word) {
	Token token = { INTEGER , { -1, -1}, ""};
	if (word == ":") {
		token.type  = OPERATOR;
		token.value = std::to_string(ASSIGN);
		shouldBeIdentifier = true;
	}else if(word == "print"){
		token.type  = KEYWORD;
		token.value = std::to_string(PRINT);
		
	}else{
		if (isValidNumber(word)) {
			token.type  = INTEGER;
			token.value = word;
		}
		else {
			if(word[0] == '\"'){
				token.type  = STRING;
				token.value = word.substr(1, word.size() - 2);
				return token;
			}

			if(shouldBeIdentifier){
				token.type = IDENTIFIER;
				token.value = word;
				shouldBeIdentifier = false;
				identifiers.push_back(word);
				return token;
			}

			for(string idn : identifiers){
				if (idn == word){
					token.type = IDENTIFIER;
					token.value = word;
					return token;
				}
			}

			printerr ("invalid token " << word);
			exit(-1);
		}
	}

	return token;
}

vector<Token> tokenizeLine(string line) {

	vector<Token> tokens;

	line += " ";
	string word = "";

	for (int i=0; i < (int)line.size(); i++){
		char ch = line[i];
		
		if (ch == '\"'){
			if (word != "") {
				printerr("syntax error!");
				exit(-1);
			}

			word += line[i];
			while(i < (int)line.size()){
				word += line[++i];
				if(line[i] == '\"' || line[i] == '\n'){
					//TODO: string is found now check validity of the string
					Token token = tokenizeWord(word);
					tokens.push_back(token);
					word = "";
					break;
				}
			}

		}
		else if (ch == 32 || ch == '\t' || ch == ',' || ch == '\n'){

			if (word.size() == 0) continue;
			if (word.rfind("//", 0) == 0) return tokens;	

			Token token = tokenizeWord(word);
			tokens.push_back(token);
			word = "";

		}else {
			word += ch;
		}
	}

	return tokens;
}

Chunk pop(vector<Chunk>& mem){
	Chunk ch = mem[mem.size() - 1];
	mem.pop_back();
	return ch;
}

void viewContext(const Context& context) {
    const int columnWidth = 20;

    cout << "Floats:" << endl;
    cout << "| " << setw(columnWidth) << "Name" << " | " << setw(columnWidth) << "Value" << " |" << endl;
    cout << "|_" << string(columnWidth, '_') << "_|_" << string(columnWidth, '_') << "_|" << endl;
    for (const auto& f : context.floats) {
        cout << "| " << setw(columnWidth) << f.name << " | " << setw(columnWidth) << f.value << " |" << endl;
    }
    cout << endl;

    cout << "Integers:" << endl;
    cout << "| " << setw(columnWidth) << "Name" << " | " << setw(columnWidth) << "Value" << " |" << endl;
    cout << "|_" << string(columnWidth, '_') << "_|_" << string(columnWidth, '_') << "_|" << endl;
    for (const auto& i : context.integers) {
        cout << "| " << setw(columnWidth) << i.name << " | " << setw(columnWidth) << i.value << " |" << endl;
    }
    cout << endl;

    cout << "Strings:" << endl;
    cout << "| " << setw(columnWidth) << "Name" << " | " << setw(columnWidth) << "Value" << " |" << endl;
    cout << "|_" << string(columnWidth, '_') << "_|_" << string(columnWidth, '_') << "_|" << endl;
    for (const auto& s : context.strings) {
        cout << "| " << setw(columnWidth) << s.name << " | " << setw(columnWidth) << s.value << " |" << endl;
    }
    cout << endl;

    cout << "Memory:" << endl;
    cout << "| " << setw(columnWidth) << "Type" << " | " << setw(columnWidth) << "Value" << " |" << endl;
    cout << "|_" << string(columnWidth, '_') << "_|_" << string(columnWidth, '_') << "_|" << endl;
    for (const auto& m : context.memory) {
        cout << "| " << setw(columnWidth) << m.type << " | " << setw(columnWidth) << m.value << " |" << endl;
    }
    cout << endl;
}

void viewProgram(vector<Token> program){
	const int columnWidth = 20;
    cout << "Program:" << endl;
    cout << "| " << setw(columnWidth) << "Type" << " | " << setw(columnWidth) << "Value" << " |" << endl;
    cout << "|_" << string(columnWidth, '_') << "_|_" << string(columnWidth, '_') << "_|" << endl;
    for (const auto& f : program) {
        cout << "| " << setw(columnWidth) << f.type << " | " << setw(columnWidth) << f.value << " |" << endl;
    }
    cout << endl;
}

void execute(vector<Token> program){

	Context context;

	for(int i=0; i < (int)program.size(); ++i){
		Token token = program[i];
		if (token.type == INTEGER || token.type == STRING){
			Chunk chunk = {token.type, token.value};
			context.memory.push_back(chunk);
		}
		if (token.type == IDENTIFIER){
			for(auto var: context.strings) {
				if (var.name == token.value){
					Chunk chunk = {token.type, var.value};
					context.memory.push_back(chunk);
				}
			}
			for(auto var: context.floats) {
				if (var.name == token.value){
					Chunk chunk = {token.type, std::to_string(var.value)};
					context.memory.push_back(chunk);
				}
			}
			for(auto var: context.integers) {
				if (var.name == token.value){
					Chunk chunk = {token.type, std::to_string(var.value)};
					context.memory.push_back(chunk);
				}
			}
		}
		if (token.type == OPERATOR){
			OperatorCode oc = (OperatorCode)std::stoi(token.value);
			Token nextToken = program[++i];
			switch (oc){
				case ASSIGN: {
					if (nextToken.type == IDENTIFIER){
						//TODO: check if the identifier is valid
						Chunk prevData = pop(context.memory);
						if (prevData.type == STRING){
							String str = {.name=nextToken.value, .value=prevData.value};
							context.strings.push_back(str);
						}else if(prevData.type == FLOAT){
							Float var = {.name= nextToken.value, .value= std::stof(prevData.value)};
							context.floats.push_back(var);
						}else if(prevData.type == INTEGER){
							Integer var = {.name= nextToken.value, .value= (int)std::stoi(prevData.value)};
							context.integers.push_back(var);
						}
					}
					break;
				}
				default:{
					printerr("Invalid operator encountered!");
					exit(-1);
				}
			}
		}
		if(token.type == KEYWORD){
			KeywordCode kc = (KeywordCode)std::stoi(token.value);
			switch(kc){
				case PRINT:{
					if (context.memory.size() <= 0) {
						printerr("No data available to print");
						exit(-1);
					}
					Chunk prevData = pop(context.memory);
					println(prevData.value);
					break;	
				}
			}
		}
#if debug
		viewContext(context);
#endif
	}
	
}

int main(int argc, char** argv){

	string filename = argv[1];

	std::ifstream file(filename);

	string line;

	vector<Token> program;
	while(getline(file, line)) {
		vector<Token> tokens = tokenizeLine(line);
		for(Token token : tokens) program.push_back(token);
	}

#if debug
	viewProgram(program);
#endif
	execute(program);
}
