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
	BOOL,
	IDENTIFIER,
	OPERATOR,
	KEYWORD,
};

enum OperatorCode {
	ASSIGN,
	ADD,
	MINUS,
	MULTIPLY,
	DIVIDE,
	MOD,
	EQUALS,
	GT,
	ST,
	GTE,
	STE
};

enum KeywordCode {
	PRINT,
	FALSE,
	TRUE,
	IF,
	ELSE,
	END
};

struct Token {
	TokenType type;

	struct Position {
		short row, col;
	};

	Position position;
	int pair; //this contains position of pair of this keyword
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

bool isValidFloat(std::string& str)
{
	for (char ch : str) {
	    int v = ch;
	    if (!(ch >= 48 && ch <= 57 || ch == '.')) {
		return false;
	    }
	}
	return true;
}

vector<string> identifiers;
bool shouldBeIdentifier = false;
Token tokenizeWord(string word) {
	Token token = { INTEGER , { -1, -1}, -1, ""};
	if (word == ":") {
		token.type  = OPERATOR;
		token.value = std::to_string(ASSIGN);
		shouldBeIdentifier = true;
	}else if(word == "print"){
		token.type  = KEYWORD;
		token.value = std::to_string(PRINT);
	}else if(word == "+"){
		token.type  = OPERATOR;
		token.value = std::to_string(ADD);
	}else if(word == "-"){
		token.type  = OPERATOR;
		token.value = std::to_string(MINUS);
	}else if(word == "*"){
		token.type  = OPERATOR;
		token.value = std::to_string(MULTIPLY);
	}else if(word == "/"){
		token.type  = OPERATOR;
		token.value = std::to_string(DIVIDE);
	}else if(word == "%"){
		token.type  = OPERATOR;
		token.value = std::to_string(MOD);
	}else if(word == "=="){
		token.type  = OPERATOR;
		token.value = std::to_string(EQUALS);
	}else if(word == "<"){
		token.type  = OPERATOR;
		token.value = std::to_string(ST);
	}else if(word == ">"){
		token.type  = OPERATOR;
		token.value = std::to_string(GT);
	}else if(word == "<="){
		token.type  = OPERATOR;
		token.value = std::to_string(STE);
	}else if(word == ">="){
		token.type  = OPERATOR;
		token.value = std::to_string(GTE);
	}else if(word == "if"){
		token.type  = KEYWORD;
		token.value = std::to_string(IF);
	}else if(word == "else"){
		token.type  = KEYWORD;
		token.value = std::to_string(ELSE);
	}else if(word == "end"){
		token.type  = KEYWORD;
		token.value = std::to_string(END);
	}else if(word == "true"){
		token.type  = KEYWORD;
		token.value = std::to_string(TRUE);
	}else if(word == "false"){
		token.type  = KEYWORD;
		token.value = std::to_string(FALSE);
	}else{
		if (isValidNumber(word)) {
			token.type  = INTEGER;
			token.value = word;
		}else if(isValidFloat(word)){
			token.type  = FLOAT;
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

void crossReference(vector<Token>& program){
	for(int i=0; i < (int)program.size(); ++i){
		Token token = program[i];
		if (token.type != KEYWORD){
			continue;
		}
		switch((KeywordCode)std::stoi(token.value)){
			case IF:{
				int index = i + 1;
				int innerBlocks = 0;

				while(index < (int)program.size()){
					Token newToken = program[index];
					if (newToken.type != KEYWORD){
						index++;
						continue;
					}
					KeywordCode code = (KeywordCode) std::stoi(newToken.value);
					if ((code == END || code == ELSE) && innerBlocks == 0){
						program[i].pair = index;
						break;
					}else {
						if (code == IF){
							innerBlocks += 1;
						}
						if (code == END){
							innerBlocks -+ 1;
						}
					}
					index++;
				}

				if(innerBlocks > 0){
					printerr("Missing END!");
					exit(-1);
				}
			}
			break;
			case ELSE:{
				int index = i+1;
				int innerBlocks = 0;

				while(index < (int)program.size()){
					Token newToken = program[index];
					if (newToken.type != KEYWORD){
						index++;
						continue;
					}
					KeywordCode code = (KeywordCode) std::stoi(newToken.value);
					if ((code == END) && innerBlocks == 0){
						program[i].pair = index;
					}else {
						if (code == IF){
							innerBlocks += 1;
						}
						if (code == END){
							innerBlocks -+ 1;
						}
					}
					index++;
				}

				if(innerBlocks > 0){
					printerr("Missing END!");
					exit(-1);
				}
			break;
			}
		}
	}
}

void viewContext(const Context& context) {
    const int columnWidth = 20;

    auto tokenTypeToString = [](TokenType type) -> string {
        switch (type) {
            case INTEGER: return "INTEGER";
            case FLOAT: return "FLOAT";
            case STRING: return "STRING";
            case BOOL: return "BOOL";
            case IDENTIFIER: return "IDENTIFIER";
            case OPERATOR: return "OPERATOR";
            case KEYWORD: return "KEYWORD";
            default: return "UNKNOWN";
        }
    };

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
        cout << "| " << setw(columnWidth) << tokenTypeToString(m.type) << " | " << setw(columnWidth) << m.value << " |" << endl;
    }
    cout << endl;
}

void viewProgram(const vector<Token>& program) {
    const int columnWidth = 20;

    auto tokenTypeToString = [](TokenType type) -> string {
        switch (type) {
            case INTEGER: return "INTEGER";
            case FLOAT: return "FLOAT";
            case STRING: return "STRING";
            case BOOL: return "BOOL";
            case IDENTIFIER: return "IDENTIFIER";
            case OPERATOR: return "OPERATOR";
            case KEYWORD: return "KEYWORD";
            default: return "UNKNOWN";
        }
    };

    cout << "Program:" << endl;
    cout << "| " << setw(columnWidth) << "Type" << " | " << setw(columnWidth) << "Value" << " |" << setw(columnWidth) << "Pair" << " |" << endl;
    cout << "|_" << string(columnWidth, '_') << "_|_" << string(columnWidth, '_') << "_|_" << string(columnWidth, '_') << "_|" << endl;
    for (const auto& token : program) {
        cout << "| " << setw(columnWidth) << tokenTypeToString(token.type) << " | " << setw(columnWidth) << token.value << " |" << setw(columnWidth) << token.pair << " |" << endl;
    }
    cout << endl;
}

void execute(vector<Token> program){

	Context context;

	for(int i=0; i < (int)program.size(); ++i){
		Token token = program[i];
		if (token.type == INTEGER || token.type == STRING || token.type == FLOAT){
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
			switch (oc){
				case ASSIGN: {
					Token nextToken = program[++i];
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
				case ADD: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);

					//TODO: throw error when both types are incompatible.
					if (a.type == STRING && b.type == STRING){
						auto res = b.value + a.value;
						Chunk resChunk = {.type = STRING, .value = res};
						context.memory.push_back(resChunk);
						break;
					}

					if (a.type == FLOAT || b.type == FLOAT){
						auto res = std::stof(b.value) + std::stof(a.value);
						Chunk resChunk = {.type = FLOAT, .value = std::to_string(res)};
						context.memory.push_back(resChunk);
						break;
					}

					auto res = std::stoi(b.value) + std::stoi(a.value);
					Chunk resChunk = {.type = INTEGER, .value = std::to_string(res)};
					context.memory.push_back(resChunk);
					break;

				}
				case MINUS: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);

					if (a.type == STRING && b.type == STRING){
						printerr("Cannot subtract string");
						exit(-1);
					}

					if (a.type == FLOAT || b.type == FLOAT){
						auto res = std::stof(b.value) - std::stof(a.value);
						Chunk resChunk = {.type = FLOAT, .value = std::to_string(res)};
						context.memory.push_back(resChunk);
						break;
					}

					auto res = std::stoi(b.value) - std::stoi(a.value);
					Chunk resChunk = {.type = INTEGER, .value = std::to_string(res)};
					context.memory.push_back(resChunk);
					break;

				}
				case MULTIPLY: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);

					if (a.type == STRING && b.type == STRING){
						printerr("Cannot multiply string");
						exit(-1);
					}

					if (a.type == FLOAT || b.type == FLOAT){
						auto res = std::stof(b.value) * std::stof(a.value);
						Chunk resChunk = {.type = FLOAT, .value = std::to_string(res)};
						context.memory.push_back(resChunk);
						break;
					}

					auto res = std::stoi(b.value) * std::stoi(a.value);
					Chunk resChunk = {.type = INTEGER, .value = std::to_string(res)};
					context.memory.push_back(resChunk);
					break;

				}
				case DIVIDE: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);

					if (a.type == STRING && b.type == STRING){
						printerr("Cannot divide string!");
						exit(-1);
					}

					if(std::stof(a.value) == 0){
						printerr("Cannot divide by zero!");
					}

					if (a.type == FLOAT || b.type == FLOAT){
						auto res = std::stof(b.value) / std::stof(a.value);
						Chunk resChunk = {.type = FLOAT, .value = std::to_string(res)};
						context.memory.push_back(resChunk);
						break;
					}

					auto res = std::stoi(b.value) / std::stoi(a.value);
					Chunk resChunk = {.type = INTEGER, .value = std::to_string(res)};
					context.memory.push_back(resChunk);
					break;

				}
				case MOD: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);

					if (a.type == STRING && b.type == STRING){
						printerr("Cannot divide string!");
						exit(-1);
					}

					if(std::stof(a.value) == 0){
						printerr("Cannot divide by zero!");
						exit(-1);
					}

					if (a.type == FLOAT || b.type == FLOAT){
						printerr("Cannot mod floats!");
					}

					auto res = std::stoi(b.value) % std::stoi(a.value);
					Chunk resChunk = {.type = INTEGER, .value = std::to_string(res)};
					context.memory.push_back(resChunk);
					break;

				}
				case EQUALS: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);
					Chunk resChunk = {.type = BOOL, .value = std::to_string(FALSE)};

					//TODO: throw error when both types are incompatible.
					if (a.type == STRING && b.type == STRING){
						resChunk.value = b.value == a.value ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					if (a.type == FLOAT || b.type == FLOAT){
						resChunk.value = std::stof(b.value) == std::stof(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					resChunk.value = std::stoi(b.value) == std::stoi(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
					context.memory.push_back(resChunk);
					break;
				}
				case GT: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);
					Chunk resChunk = {.type = BOOL, .value = std::to_string(FALSE)};

					//TODO: throw error when both types are incompatible.
					if (a.type == STRING && b.type == STRING){
						resChunk.value = b.value > a.value ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					if (a.type == FLOAT || b.type == FLOAT){
						resChunk.value = std::stof(b.value) > std::stof(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					resChunk.value = std::stoi(b.value) > std::stoi(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
					context.memory.push_back(resChunk);
					break;
				}
				case ST: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);
					Chunk resChunk = {.type = BOOL, .value = std::to_string(FALSE)};

					//TODO: throw error when both types are incompatible.
					if (a.type == STRING && b.type == STRING){
						resChunk.value = b.value < a.value ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					if (a.type == FLOAT || b.type == FLOAT){
						resChunk.value = std::stof(b.value) < std::stof(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					resChunk.value = std::stoi(b.value) < std::stoi(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
					context.memory.push_back(resChunk);
					break;
				}
				case GTE: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);
					Chunk resChunk = {.type = BOOL, .value = std::to_string(FALSE)};

					//TODO: throw error when both types are incompatible.
					if (a.type == STRING && b.type == STRING){
						resChunk.value = b.value >= a.value ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					if (a.type == FLOAT || b.type == FLOAT){
						resChunk.value = std::stof(b.value) >= std::stof(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					resChunk.value = std::stoi(b.value) >= std::stoi(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
					context.memory.push_back(resChunk);
					break;
				}
				case STE: {
					Chunk a = pop(context.memory);
					Chunk b = pop(context.memory);
					Chunk resChunk = {.type = BOOL, .value = std::to_string(FALSE)};

					//TODO: throw error when both types are incompatible.
					if (a.type == STRING && b.type == STRING){
						resChunk.value = b.value <= a.value ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					if (a.type == FLOAT || b.type == FLOAT){
						resChunk.value = std::stof(b.value) <= std::stof(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
						context.memory.push_back(resChunk);
						break;
					}

					resChunk.value = std::stoi(b.value) <= std::stoi(a.value) ? std::to_string(TRUE) : std::to_string(FALSE);
					context.memory.push_back(resChunk);
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

					if(prevData.type == BOOL){
						if(std::stoi(prevData.value) == TRUE)
							println("true");
						else
							println("false");
						break;
					}
					println(prevData.value);
					break;	
				}
				case TRUE:{
					Chunk chunk = {BOOL, std::to_string(TRUE)};
					context.memory.push_back(chunk);
					break;
				}
				case FALSE:{
					Chunk chunk = {BOOL, std::to_string(FALSE)};
					context.memory.push_back(chunk);
					break;
				}
				case IF: {
					Chunk prevData = pop(context.memory);
					if (prevData.type == BOOL){
						if(std::stoi(prevData.value) == TRUE){
							// do nothing i guess
						}else{
							i = token.pair;
							continue;
						}
					}
					break;
				}
				case ELSE:{
					i = token.pair; 
					break;
				}
				case END:{
					break;
				}
				default:{
					printerr("how did unrecognised keyword came here?");
					exit(-1);
				}
			}
		}
	}
#if debug
		viewContext(context);
#endif

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

	crossReference(program);
	execute(program);
#if debug
	viewProgram(program);
#endif
}
