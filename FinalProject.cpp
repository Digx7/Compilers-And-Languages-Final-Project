// Final Project for Compilers
// By: Abraham and Everette
// This program is ment to be a simple compiler


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <algorithm>
#include <stdlib.h>
#include <cstdlib>
using namespace std;

const string LINE1 = "====================================\n";
const string LINE2 = "------------------------------------\n";
const string AUTHORS = "Everette & Abraham";


// UI Functions ===================================
bool getYesOrNoResponse(string exitMessage){
    while( 1 > 0){
        string input = "";
        std::cout << exitMessage << "(y/n): ";
        std::cin >> input;

        if (input == "N"||input == "n") return false;
        else if(input == "Y"||input == "y") return true;

        std::cout << "I'm sorry I did not recognize that input\nPlease try again" << std::endl;
    }
}

bool doesFileExist(string fileName){
    ifstream f(fileName);
    return f.good();
}

void welcomeMessage(){
    std::cout << LINE1 << "This is a general compiler for the final project using Predictive Parsing Table\n" << "Designed by: " << AUTHORS << "\n" << LINE1 << flush;
}

void goodByeMessage(){
    std::cout << "Good Bye :)" << std::endl << flush;
}

string getStringFromUser(string promptMessage){
    cout << promptMessage << ": ";
    string output;
    cin >> output;
    return output;
}

string getFileNameFromUser(string promptMessage, string fileExtenstion, bool requireFileToExist = false){

    // Initialize variables
    string output;
    string confirmMessage;
    bool confirm;

    while(1 > 0){ // will keep looping until the user confirms
        output = getStringFromUser(promptMessage);
        string fullFileName = output + fileExtenstion;
        confirmMessage = "Confirm '" + fullFileName + "' ";
        confirm = getYesOrNoResponse(confirmMessage);
        if(confirm){
            if(requireFileToExist && !doesFileExist(fullFileName)) cout << "I'm sorry " << fullFileName << " was not found" << endl;
            else return output;
        }
    }
}

// Utillity Functions =============================
string addSpaceAroundCharacter(string str, char character, char stoppingCharacter)
{
    // Initialize variables
    bool canEdit = true;
    string output = "";  // a new string we will write to

    for(int i = 0; i < str.length(); i++) { // iterates through each character in the string
        
        if(str[i]==stoppingCharacter) // if we see the stoppingCharacter we can't add anymore spaces
            canEdit = false;

        if(canEdit && str[i]==character)  // if we found the character add spaces around it
            output = output + " " + str[i] + " ";  
        else  // just copy over the string one for one
            output = output + str[i];
    }
    return output;
}

int findStringInVector(vector<string> mainVector, string stringToLookFor){
    
    // Iterates through our vector looking for stringToLookfor
    auto it = find(mainVector.begin(), mainVector.end(), stringToLookFor);
  
    // If element was found
    if (it != mainVector.end()) 
    {
      
        // calculating the index of stringToLookFor
        int index = it - mainVector.begin();
        return index;
    }
    else {
        // If the element is not present in the vector
        return -1;
    }
}

// Classes =======================================
class language{

    public:
        vector<vector<string>> table;
        vector<string> terminals;
        vector<string> nonTerminals;
        vector<string> reservedWords;

    public:

        language(){}

        language(vector<vector<string>> _table, vector<string> _terminals, vector<string> _nonTerminals, vector<string> _reservedWords){
            table = _table;
            terminals = _terminals;
            nonTerminals = _nonTerminals;
            reservedWords = _reservedWords;
        }

        bool isReservedWord(string input){
            for(int i = 0; i < reservedWords.size(); ++i ){
                if(input == reservedWords[i]) return true;
            }
            return false;
        }

        string GoTo(string nonTerminal, string terminal){
            // Gets the int index of the nonTerminal
            int index_nonTerminal = findStringInVector(nonTerminals, nonTerminal);

            // Gets the int index of the terminal
            int index_terminal = findStringInVector(terminals, terminal);

            // Checks if either index is out of bounds
            if(index_nonTerminal == -1 || index_terminal == -1)return "E_Out of Bounds";

            // Returns the value in the table at the given indexes
            return table[index_nonTerminal][index_terminal];
        }

};

class compiler{

    private:
        stack<string> parsingStack;
        string currentInputToken;
        string currentPoppedElement;

        vector<string> definedIdentifiers;
        vector<char> currentIdentifier;

        bool isInDebugMode;
        bool isLookingAtIdentifier;
        bool isDefiningIdentifiers;

        int currentLineNumber;
        int currentTokenNumber;
        int maxNumberOfLines;

    public:
        language chosenLanguage;
        string inputFileName;
        vector<string> errors;

    private: 

        void resetCompiler(){
            while(!parsingStack.empty())
                parsingStack.pop();

            currentInputToken = "";
            currentPoppedElement = "";
            currentLineNumber = 1;
            currentTokenNumber = 1;
            if(isInDebugMode) cout << "Compiler reset" << endl;

            parsingStack.push("<prog>");
            if(isInDebugMode) cout << "Push: <prog>" << endl;
        }

        bool lookingForMatch(string input){
            bool matchFound = false;

            while(!matchFound){
                if(isInDebugMode) cout << LINE2;
                //Pop top element from stack
                popFromStack();
                //if top element == currentInputToken then matchFound = true
                if(currentPoppedElement == input){
                    matchFound = true;
                    if(isInDebugMode) cout << "MATCH FOUND: " << currentPoppedElement << " == " << input << endl;
                    if(isLookingAtIdentifier) buildIdentifier(input[0]);
                    if(currentPoppedElement == "begin") isDefiningIdentifiers = false;
                }
                else{
                    string outputString = chosenLanguage.GoTo(currentPoppedElement, input);
                    if(isInDebugMode) cout << "Go To [" << currentPoppedElement << " , " << input << "] = " << outputString << endl;

                    if(currentPoppedElement == "<identifier_start>") isLookingAtIdentifier = true;
                    if(isLookingAtIdentifier && outputString == "lamda") {
                        isLookingAtIdentifier = false;
                        // TODO: Make it so the identifiers are either defined or checked depending on where we are in the code
                        if(isDefiningIdentifiers) defineIdentifier();
                        else if (!isDefinedIdentifier()){
                            outputString = "E_unknown identifier";
                        }
                    }

                    if(outputString == "") {
                        outputString = "E_Unknown error";
                    }
                    if(isError(outputString)){
                        string errorMessage = defineError(outputString, input);
                        addErrorMessage(errorMessage);
                        return false;
                    }
                    if(outputString != "lamda"){
                        pushToStack(outputString);
                    }
                }
            }
            return true;
        }

        // Parsing Stack functions  
        bool popFromStack(){
            if(parsingStack.empty()) return false;

            currentPoppedElement = parsingStack.top();
            parsingStack.pop();

            if(isInDebugMode) cout << "Pop: " << currentPoppedElement << endl;

            return true;
        }

        void pushToStack(string input){
            stack<string> reversingStack;
            stringstream s(input);
            string token;

            while(s >> token) {
                reversingStack.push(token);
            }
            while(!reversingStack.empty()){
                parsingStack.push(reversingStack.top());
                reversingStack.pop();
                if(isInDebugMode) cout << "Push: " << parsingStack.top() << endl;
            }
            if(isInDebugMode) cout << "Top of Stack is: " << parsingStack.top() << endl;
        }


        // Identifier functions
        bool isDefinedIdentifier(){
            string identifier(currentIdentifier.begin(), currentIdentifier.end());
            while(!currentIdentifier.empty()) currentIdentifier.pop_back();

            if(findStringInVector(definedIdentifiers, identifier) == -1) return false;
            return true;
        }

        void defineIdentifier(){
            string identifier(currentIdentifier.begin(), currentIdentifier.end());
            definedIdentifiers.push_back(identifier);

            while(!currentIdentifier.empty()) currentIdentifier.pop_back();
        }

        void buildIdentifier(char input){
            currentIdentifier.push_back(input);
            if(isInDebugMode) cout << "Building Identifier" << endl;
        }

        // Error functions
        bool isError(string input){
            if(input[0] == 'E') return true;
        }

        string defineError(string inputString, string lastInput){

            // if we go out of bounds on our language table
            if(inputString == "E_Out of Bounds"){

                // if we tried to look for a terminal in the collumns, which are all non-terminals
                if(findStringInVector(chosenLanguage.terminals, currentPoppedElement) != -1){
                    inputString = "E_" + currentPoppedElement + " is expected";
                }

                // if we tried to use a character in our terminals that doesn't exist in our language
                else if(findStringInVector(chosenLanguage.terminals, lastInput) == -1){
                    // when we are at '<type>' the only thing we should see is 'integer' 
                    // so if we see anything else we get this error
                    if(currentPoppedElement == "<type>") 
                        inputString = "E_integer is expected";
                    else 
                        inputString = "E_" + lastInput + " is an invalid character";
                        if(lastInput == "e" || lastInput == "E") inputString = inputString + " \ndid you misspell 'end.'";
                        if(lastInput == "d" || lastInput == "D") inputString = inputString + " \ndid you misspell 'display'";
                }
            }

            // checks if we are at the end of the file
            // will trigger if end. is missspelled
            if(currentLineNumber == maxNumberOfLines) inputString = "E_end. is expected";

            return inputString;
        }

        void addErrorMessage(string errorMessage){
            string message = errorMessage.substr(2, errorMessage.size() - 2);
            string location = "[" + to_string(currentLineNumber) + "," + to_string(currentTokenNumber) + "]"; 
            string finalMessage = location + " " + message;
            errors.push_back(finalMessage);
        }

    public:
        compiler(language _language, string _inputFileName, int _maxNumberOfLines){
            // Assigning variables
            chosenLanguage = _language;
            inputFileName = _inputFileName;
            maxNumberOfLines = _maxNumberOfLines;

            // Ensures these variable start in this state
            isInDebugMode = false;
            isLookingAtIdentifier = false;
            isDefiningIdentifiers = true;
        }

        bool run(){
            resetCompiler();
            ifstream MyReadFile(inputFileName);
            string myLine;

            while(getline (MyReadFile, myLine)){
                if(isInDebugMode) cout << LINE1;
                if(isInDebugMode) cout << "Looking at line: " << myLine << endl;
                if(isInDebugMode) cout << LINE1;
                currentTokenNumber = 1;
                stringstream s(myLine);
                while(s >> currentInputToken){
                    if(isInDebugMode) cout << "Looking at token: " << currentInputToken << endl;

                    bool tokenIsReserveWord = chosenLanguage.isReservedWord(currentInputToken);

                    if(!tokenIsReserveWord){
                        if(isInDebugMode) cout << "Token: " << currentInputToken << " is NOT a reserved word and is being split up" << endl;
                        for(int i = 0; i < currentInputToken.length(); ++i){
                            string tmp = string(1, currentInputToken[i]);
                            if(!lookingForMatch(tmp)) return false;
                        }
                    }
                    else{
                        if(isInDebugMode) cout << "Token: " << currentInputToken << " is a reserved word and is being treated like one character" << endl;
                        if(!lookingForMatch(currentInputToken)) return false;
                    }
                    currentTokenNumber++;
                }
                currentLineNumber++;
            }
            MyReadFile.close();
            if(currentInputToken == "end."){
                return true;
            }
            else{
                // if we got to the end of the file but don't see 'end.'
                addErrorMessage("E_end. is expected");
                return false;
            }
        }

        void flushParsingStack(){
            if(isInDebugMode) cout << "Items left in parsing stack: " << endl;
            while(!parsingStack.empty()){
                if(isInDebugMode) cout << parsingStack.top() << endl;
                parsingStack.pop();
            }
            if(isInDebugMode) cout << "End of parsing stack" << endl;

        }

        void displayAllDefinedIdentifiers(){
        cout << "All Defined Identifiers: " << endl;
        for(int i = 0; i < definedIdentifiers.size(); ++i){
            cout << definedIdentifiers[i] << endl;
        }
    }

        void setDebugMode(bool input){
            isInDebugMode = input;
        }

};

// Part 1 - 4 Functions ==========================
int part1(string readFileName, string writeFileName, bool useDebugMode = false){
    // turn the text file "finalp1.txt" into "finalp2.txt"

    if(useDebugMode){
        cout << LINE1;
        cout << "PART 1" << endl;
        cout << "Attempting to write a cleaned up version of " << readFileName << " in " << writeFileName << endl;
    }

    // Open files
    ifstream MyReadFile(readFileName);
    ofstream MyWriteFile(writeFileName);

    // Initialize variables
    bool shouldWrite = true;
    bool wordWasWritten = false;
    int numberOfLines = 0;
    string myLine;

    while(getline (MyReadFile, myLine)){ // lets us view each line of our read file
        // Initialize variables
        stringstream s(myLine); 
        string word;

        while(s >> word){ // lets us view each word of each line of our read file
            // Initialize variables
            string dictonary = ":;,=+-()";   // characters of words to add spaces around
            char stoppingChar = '"';         // stopping character so we don't mess up strings

            for(int i=0; i<dictonary.length(); ++i){ // for each character in the above dictionary, if its in the word we're looking at then add a space around it
                word = addSpaceAroundCharacter(word,dictonary[i],stoppingChar);
            }
            if(word[0] == '/' && word[1] == '/') shouldWrite = false;  // So we don't write comments
            if(shouldWrite){ // Writes the current word to the write file
                MyWriteFile << word << ' ';
                wordWasWritten = true;
            }
        }

        if(wordWasWritten) { // allows us to remove blank lines
            MyWriteFile << "\n";     
            numberOfLines++;
        }
        
        // Reset variables for next loop
        shouldWrite = true;
        wordWasWritten = false;
    }

    // Closes files 
    MyReadFile.close();
    MyWriteFile.close();

    if(useDebugMode){
        cout << writeFileName << " should now be a cleaned up version of " << readFileName << endl;
        cout << "You can check " << writeFileName << " now or press any key to let the program continue" << endl << flush;
        system("PAUSE");
    }

    return numberOfLines; // Is used the compiler for some error checking
}

vector<string> part2(string orginalFileName ,string readFileName, int maxNumberOfLines, bool useDebugMode = false){
    // run the "finalp2.txt" through the grammer to check for errors

    if(useDebugMode){
        cout << LINE1;
        cout << "PART 2" << endl;
        cout << "Attempting to trace the cleaned up version of " << orginalFileName << " in " << readFileName << " to see if it is valid" << endl;
        cout << "Press any key to beging tracing " << readFileName << endl << flush;
        system("PAUSE");
        cout << LINE2;
    }

    // Our langage table
    vector<vector<string>> table
    {
        //                           program                                                                     var                      begin                     end.                    integer                  display                     ;                        :                        ,                        "value="                               (                               )                        =                        +                               -                               *                            /                            0                            1                            2                            3                            4                            5                            6                            7                             8                             9                             p                                                  q                                                  r                                                  s                                      
        /*<prog>                  */{"program <identifier_start> ; var <dec-list> begin <stat-list_enter> end.", "E_program is expected", "E_program is expected", "E_program is expected", "E_program is expected", "E_program is expected",    "E_program is expected", "E_program is expected", "E_program is expected", "E_program is expected",               "E_program is expected",        "E_program is expected", "E_program is expected", "E_program is expected",        "E_program is expected",        "E_program is expected",     "E_program is expected",     "E_program is expected",     "E_program is expected",     "E_program is expected",     "E_program is expected",     "E_program is expected",     "E_program is expected",     "E_program is expected",     "E_program is expected",      "E_program is expected",      "E_program is expected",      "E_program is expected",                           "E_program is expected",                           "E_program is expected",                           "E_program is expected"},
        /*<identifier_start>      */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<letter> <identifier_body>",                      "<letter> <identifier_body>",                      "<letter> <identifier_body>",                      "<letter> <identifier_body>"},
        /*<identifier_body>       */{"",                                                                         "E_; is expected",       "",                       "",                     "",                      "E_; is expected",          "lamda",                 "lamda",                 "lamda",                 "",                                    "E_( is an invalid character",  "lamda",                 "lamda",                 "lamda",                        "lamda",                        "lamda",                     "lamda",                     "<digit> <identifier_body>", "<digit> <identifier_body>", "<digit> <identifier_body>", "<digit> <identifier_body>", "<digit> <identifier_body>", "<digit> <identifier_body>", "<digit> <identifier_body>", "<digit> <identifier_body>",  "<digit> <identifier_body>",  "<digit> <identifier_body>",  "<letter> <identifier_body>",                      "<letter> <identifier_body>",                      "<letter> <identifier_body>",                      "<letter> <identifier_body>"},                                          
        /*<dec-list>              */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<dec_enter> : <type> ;",                          "<dec_enter> : <type> ;",                          "<dec_enter> : <type> ;",                          "<dec_enter> : <type> ;"},
        /*<dec_enter>             */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<identifier_start> <dec>",                        "<identifier_start> <dec>",                        "<identifier_start> <dec>",                        "<identifier_start> <dec>"},
        /*<dec>                   */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "lamda",                 ", <dec_enter>",         "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "",                                                "",                                                "",                                                ""},
        /*<type>                  */{"E_integer is expected",                                                    "E_integer is expected", "E_integer is expected", "E_integer is expected", "integer",               "E_integer is expected",    "E_integer is expected", "E_integer is expected", "E_integer is expected", "E_integer is expected",               "E_integer is expected",        "E_integer is expected", "E_integer is expected", "E_integer is expected",        "E_integer is expected",        "E_integer is expected",     "E_integer is expected",     "E_integer is expected",     "E_integer is expected",     "E_integer is expected",     "E_integer is expected",     "E_integer is expected",     "E_integer is expected",     "E_integer is expected",     "E_integer is expected",      "E_integer is expected",      "E_integer is expected",      "E_integer is expected",                           "E_integer is expected",                           "E_integer is expected",                           "E_integer is expected"},
        /*<stat-list_enter>       */{"",                                                                         "",                      "",                       "lamda",                "",                      "<stat> <stat-list>",       "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<stat> <stat-list>",                              "<stat> <stat-list>",                              "<stat> <stat-list>",                              "<stat> <stat-list>"},
        /*<stat-list>             */{"",                                                                         "",                      "",                       "lamda",                "",                      "<stat> <stat-list_enter>", "",                      "",                      "",                      "",                                    "E_display is expected",        "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<stat> <stat-list_enter>",                        "<stat> <stat-list_enter>",                        "<stat> <stat-list_enter>",                        "<stat> <stat-list_enter>"},
        /*<stat>                  */{"",                                                                         "",                      "",                       "",                     "",                      "<write_enter>",            "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<assign>",                                        "<assign>",                                        "<assign>",                                        "<assign>"},
        /*<write_enter>           */{"",                                                                         "",                      "",                       "",                     "",                      "display ( <write>",        "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "",                                                "",                                                "",                                                ""},
        /*<write>                 */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "E_unexpected ,",        "\"value=\" , <identifier_start> ) ;", "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<identifier_start> ) ;",                          "<identifier_start> ) ;",                          "<identifier_start> ) ;",                          "<identifier_start> ) ;"},
        /*<assign>                */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "<identifier_start> = <expr_term_factor_enter> ;", "<identifier_start> = <expr_term_factor_enter> ;", "<identifier_start> = <expr_term_factor_enter> ;", "<identifier_start> = <expr_term_factor_enter> ;"},
        /*<expr_term_factor_enter>*/{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "<term_enter> <expr>",          "",                      "",                      "<term_enter> <expr>",          "<term_enter> <expr>",          "",                          "",                          "<term_enter> <expr>",       "<term_enter> <expr>",       "<term_enter> <expr>",       "<term_enter> <expr>",       "<term_enter> <expr>",       "<term_enter> <expr>",       "<term_enter> <expr>",       "<term_enter> <expr>",        "<term_enter> <expr>",        "<term_enter> <expr>",        "<term_enter> <expr>",                             "<term_enter> <expr>",                             "<term_enter> <expr>",                             "<term_enter> <expr>"},
        /*<expr>                  */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "lamda",                 "",                      "",                      "",                                    "",                             "lamda",                 "",                      "+ <term_enter> <expr>",        "- <term_enter> <expr>",        "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "",                                                "",                                                "",                                                ""},
        /*<term_enter>            */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "<factor> <term>",              "",                      "",                      "<factor> <term>",              "<factor> <term>",              "",                          "",                          "<factor> <term>",           "<factor> <term>",           "<factor> <term>",           "<factor> <term>",           "<factor> <term>",           "<factor> <term>",           "<factor> <term>",           "<factor> <term>",            "<factor> <term>",            "<factor> <term>",            "<factor> <term>",                                 "<factor> <term>",                                 "<factor> <term>",                                 "<factor> <term>"},
        /*<term>                  */{"",                                                                         "",                      "",                       "",                     "",                      "E_; is expected",          "lamda",                 "",                      "",                      "",                                    "",                             "lamda",                 "",                      "lamda",                        "lamda",                        "* <factor> <term>",         "/ <factor <term>",          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "",                                                "",                                                "",                                                ""},
        /*<factor>                */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "( <expr_term_factor_enter> )", "",                      "",                      "<number_start>",               "<number_start>",               "",                          "",                          "<number_start>",            "<number_start>",            "<number_start>",            "<number_start>",            "<number_start>",            "<number_start>",            "<number_start>",            "<number_start>",             "<number_start>",             "<number_start>",             "<identifier_start>",                              "<identifier_start>",                              "<identifier_start>",                              "<identifier_start>"},
        /*<number_start>          */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "<sign> <digit> <number_body>", "<sign> <digit> <number_body>", "",                          "",                          "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",      "<digit> <number_body>",      "<digit> <number_body>",      "",                                                "",                                                "",                                                ""},
        /*<number_body>           */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "lamda",                 "",                      "",                      "",                                    "",                             "lamda",                 "",                      "lamda",                        "lamda",                        "lamda",                     "lamda",                     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",     "<digit> <number_body>",      "<digit> <number_body>",      "<digit> <number_body>",      "E_can't use letters in number",                   "E_can't use letters in number",                   "E_can't use letters in number",                   "E_can't use letters in number"},
        /*<sign>                  */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "+",                            "-",                            "",                          "",                          "lamda",                     "lamda",                     "lamda",                     "lamda",                     "lamda",                     "lamda",                     "lamda",                     "lamda",                      "lamda",                      "lamda",                      "",                                                "",                                                "",                                                ""},
        /*<digit>                 */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "0",                         "1",                         "2",                         "3",                         "4",                         "5",                         "6",                         "7",                          "8",                          "9",                          "",                                                "",                                                "",                                                ""},
        /*<letter>                */{"",                                                                         "",                      "",                       "",                     "",                      "",                         "",                      "",                      "",                      "",                                    "",                             "",                      "",                      "",                             "",                             "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                          "",                           "",                           "",                           "p",                                               "q",                                               "r",                                               "s"}
    };

    // All the terminals of our language, its used for index searching later
    vector<string> terminals
    {
        "program", "var", "begin", "end.", "integer", "display", ";", ":", ",", "\"value=\"", "(", ")", "=", "+", "-", "*", "/", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "p", "q", "r", "s"
    };

    // All the Non-Terminals of our language, its used for index searching later
    vector<string> nonTerminals
    {
        "<prog>", "<identifier_start>", "<identifier_body>", "<dec-list>", "<dec_enter>", "<dec>", "<type>", "<stat-list_enter>", "<stat-list>", "<stat>", "<write_enter>", "<write>", "<assign>", "<expr_term_factor_enter>", "<expr>", "<term_enter>", "<term>", "<factor>", "<number_start>", "<number_body>", "<sign>", "<digit>", "<letter>"
    };

    // All the reserved words, its used to see if we are looking at an identifier or a reserved word
    vector<string> reservedWords
    {
        "program", "var", "begin", "end.", "integer", "display" , "\"value=\""
    };

    // Creates classes for our language and compiler
    language myLanguage(table, terminals, nonTerminals, reservedWords);
    compiler myCompiler(myLanguage, readFileName, maxNumberOfLines);

    // Runs our compiler and gets the result
    myCompiler.setDebugMode(useDebugMode);
    bool result = myCompiler.run();

    if(result) { // If it compiled display this message
        cout << "File accepted" << endl;
    }
    if(!result) { // If it failed to compile display this message
        cout << orginalFileName << " failed to compile" << endl;
        cout << "File NOT accepted" << endl;
        myCompiler.flushParsingStack();
    }

    // If in debug mode then shows you all defined identifiers
    if(useDebugMode) {
        myCompiler.displayAllDefinedIdentifiers();
        cout << "Finished tracing " << readFileName << endl;
        cout << "You can check your files or press any key and let the program continue" << endl << flush;
        system("PAUSE");
    }

    return myCompiler.errors;
}

void part3(vector<string> errors, bool useDebugMode = false){
    // display all error messages

    if(useDebugMode){
        cout << LINE1;
        cout << "PART 3" << endl;
        cout << "Attempting to display all the errors found" << endl;
    }

    if(!errors.empty()){   // if there are error messages then display them
        for(int i = 0; i < errors.size(); ++i){
            cout << "Error " << errors[i] << endl;
        }
    } else if(useDebugMode){
        cout << "No errors found" << endl;
    }
 
    if(useDebugMode){
        cout << "Finished displaying all errors" << endl << flush;
        system("PAUSE");
    }

}

void part4(string orginalFileName ,string readFileName, string writeFileName, string writeFileNameWithOutExtension, bool useDebugMode = false){
    // convert our custom programming language into c++

    if(useDebugMode){
        cout << LINE1;
        cout << "PART 4" << endl;
        cout << "Attempting to translate the cleaned up version of " << orginalFileName << " in " << readFileName << " into c++ in " << writeFileName << endl;
    }

    // initializing variables we'll need
    bool isInVarBlock = false;
    bool isInBeginBlock = false;
    bool wordWasWritten = false;
    string myLine;

    // opening files
    ifstream MyReadFile(readFileName);
    ofstream MyWriteFile(writeFileName);

    // adding c++ header to file
    string CppHeader = "#include <iostream>\nusing namespace std;\nint main()\n{\n";
    MyWriteFile << CppHeader;

    // writing to file
    while(getline (MyReadFile, myLine)){  // This lets of check each line of our readfile
        stringstream s(myLine);    // This will let of check each word in each line of our readfile
        string word;

        //if we are in the var block of code
        if(isInVarBlock){
            MyWriteFile << "int ";
            while(s >> word){
                if(word != ":" && word != "integer") MyWriteFile << word << ' ';
            }
            isInVarBlock = false;
            wordWasWritten = true;
        } else if(myLine[0] == 'd'){   // if this line has a display
            MyWriteFile << "cout << ";
            while(s >> word){
                if(word != "display" && word != "(" && word != ")" && word != "," && word != ";") MyWriteFile << word << " << ";
            }
            MyWriteFile << "endl;";
            wordWasWritten = true;
        } else{  // handles all other edge cases
            while(s >> word){
                if(word == "end.")isInBeginBlock = false; // checks if we have left the begin block
                if(isInBeginBlock){  // if we are in the begin block
                    MyWriteFile << word << ' ';
                    wordWasWritten = true;
                }

                // Checks if we are entering a block of code
                // These are at the end so the words 'var' and 'begin' don't get written
                if(word == "var") isInVarBlock = true;
                if(word == "begin") isInBeginBlock = true;
            }
        }

        if(wordWasWritten) {    // If we actually wrote a line this will and a \n at the end of the line
            MyWriteFile << "\n";
        }
        wordWasWritten = false;
    }

    // adding c++ footer to file
    string CppFooter = "return 0;\n}";
    MyWriteFile << CppFooter;

    // closing files
    MyReadFile.close();
    MyWriteFile.close();

    cout << orginalFileName << " has been compiled into " << writeFileName << endl;
    cout << "Attempting to compile " << writeFileName << " with the following command: ";
    cout << "g++ " << writeFileName << endl;

    string compileCommand = "g++ " + writeFileName;
    string runCommand = ".\\" + writeFileNameWithOutExtension + ".exe";  

    const char* compileStr = compileCommand.c_str();
    const char* runStr = runCommand.c_str();

    system(compileStr);

    if(doesFileExist(writeFileNameWithOutExtension + ".exe")){
        cout << "Attempting to run compiled file" << endl;
        system(runStr);
        cout << "Compiled file exited" << endl;
    } else{
        cout << "Failed to compile " << writeFileName << endl;
        cout << "Please try to compile it yourself once this program has exited" << endl;
    }
}


// Driver Code
int main()
{
    // Variables
    string orignialFileName, orignialFileNameWithExtenstion;
    string cleanedUpFileName = "finalp2.txt";
    string endingCppFileName, endingCppFileNameWithExtenstion;
    int numberOfLinesInCleanedUpFile;
    bool isGoing = true;
    bool isInDebugMode = false;
    vector<string> errors;

    //Main process
    welcomeMessage();

    while(isGoing){

        // Get user input
        orignialFileName = getFileNameFromUser("Please enter the name of the exiting text file you want compiled", ".txt", true);
        endingCppFileName = getFileNameFromUser("Please enter the name you want for the final C++ file", ".cpp");
        isInDebugMode = getYesOrNoResponse("Enable debug mode for this compiler? ");

        orignialFileNameWithExtenstion = orignialFileName + ".txt";
        endingCppFileNameWithExtenstion = endingCppFileName + ".cpp";

        cout << LINE2;
        cout << "Running compiler" << endl;

        numberOfLinesInCleanedUpFile = part1(orignialFileNameWithExtenstion, cleanedUpFileName, isInDebugMode);
        errors = part2(orignialFileNameWithExtenstion ,cleanedUpFileName, numberOfLinesInCleanedUpFile, isInDebugMode);
        part3(errors, isInDebugMode);
        if(errors.empty())part4(orignialFileNameWithExtenstion ,cleanedUpFileName, endingCppFileNameWithExtenstion, endingCppFileName, isInDebugMode);
        else cout << "Can not translate to C++ due to errors" << endl;

        cout << LINE2;
        isGoing = getYesOrNoResponse("Do you want to compile another file?");
        cout << LINE1;
    }

    goodByeMessage();

    return 0;
}