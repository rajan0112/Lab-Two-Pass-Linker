//##########################//
// LAB1- TWO-PASS LINKER //
// Rajan Chaturvedi //
// February 2021 //
//##########################//

#include <iostream>
#include <iomanip>
#include <map>
#include <regex>
#include<iterator>

using namespace std;

//##GLOBAL VARIABLES AND CONSTANTS DECLARATION##//

int mod_no=0; //Module Number//
int mod_count=0; //Module Count//
int line_no=1; //Line Number//
int cur_line_no=1; //Current Line Number//
int ch_count=0; //Character Count//
int line_off=0; //Line Offset//
int ch_offset=1; //Character Offset//
int tmp_no=0; //Temporary variable for number of instructions//
const int max_mem=512; //Maximum Machine Memory Size//
const int max_sym_len=16; //Maximum Symbol Length//
const int max_def_len=16; //Maximum List Count//
regex digit_num("[0-9]+");
regex symbol_type("([A-Z]|[a-z])([A-Z]|[a-z]|[0-9])*");

//################################################//

//##MAP CREATION##//

map<string,int> symbol_table;
vector<string> multiple_symbols;
map<string,int> symbol_usecount;
map<string,int> module;
vector<string> current_uselist;
map<string,int> current_definition_list;
map<string,bool> tmp_symbol;

//##################################//

//##FLAGS##//

bool flag = false;
bool second_pass_flag = false;

//###########################//

char *file_path;
FILE *input;

//##Function to Reinitialize the Global Variables, Maps##//

void reinitialize (){
	mod_no=0;
    mod_count=0;
	line_no=1;
    cur_line_no=1;
    line_off=0;
    ch_offset=1;
	tmp_no=0;
    flag = false;
    second_pass_flag=false;
    symbol_table.clear();
    module.clear();
    multiple_symbols.clear();
    symbol_usecount.clear();
    current_uselist.clear();
}

//##Function to Print the Errors##//

void print_error(int err_code) {
    const string err_string[] = {
            "NUM_EXPECTED", // Number is expected
            "SYM_EXPECTED", // Symbol is expected
            "ADDR_EXPECTED", // Addressing is expected (A/E/I/R)
            "SYM_TO_LONG", // Symbol name is too long
            "TOO_MANY_DEF_IN_MODULE", // >  16
            "TOO_MANY_USE_IN_MODULE", // >  16
            "TOO_MANY_INST", //total number of instructions exceeds the max memory size(512).
    };
    cout<<"Parse Error line "<<cur_line_no<< " offset "<<ch_offset<<": "<<err_string[err_code]<<endl;
    exit(1);
}

//#################################//

//##Function to Print the Symbol Table##//

void print_symbol_table(){
    cout<<"Symbol Table"<<endl;
    for (map<string,int>::iterator ptr=symbol_table.begin(); ptr!=symbol_table.end(); ++ptr) {
        cout << ptr->first << "=" << ptr->second;
        if(find(multiple_symbols.begin(), multiple_symbols.end(),ptr->first) != multiple_symbols.end()){
            cout<<" Error: This variable is multiple times defined; first value used";
        }
        cout<<endl;
    }
}

//#################################//

//##TOKENIZER##//

string get_tokens(){
    string str ="";
    char cur_char;
    cur_char=fgetc(input);
    ch_count++;
    if(feof(input)){
        return str;
    }
    while(cur_char!=' '&&cur_char!='\t'&&cur_char!='\n'&&!feof(input)){
        ch_count++;
        str = str+cur_char;
        cur_char=fgetc(input);
    }
    cur_line_no = line_no;
    ch_offset = ch_count;
    if(cur_char == '\n'){
        line_no++;
        ch_count=0;
    }
if(str.empty()){
        str = get_tokens();
    }
    return str;
}

//#####################//

//## Read the Integers from the Input file##//

int read_integers(){
int x = 0;
    string str = get_tokens();

    if(str.empty()){
        if(!flag) {
            return -1;
        }
        else{
            ch_offset-=str.length();
            print_error(0);
        }
    }
    if(!regex_match(str,digit_num)){
        ch_offset-=str.length();
        print_error(0);
    }
    else{
        x = stoi(str);
    }
    return x;
}

//###############################//

//## Read the Symbols from the Input file##//

string read_symbol(){
    string str = get_tokens();
    if(str.length()>max_sym_len){
        ch_offset-=str.length();
        print_error(3);
    }
    if(!regex_match(str,symbol_type)){
        ch_offset-=str.length();
        print_error(1);
    }
    return str;
}

//###############################//

//## Read the Definitions ##//

void read_definitions(){
    string st= read_symbol();
    int z=read_integers();
    if(!second_pass_flag){
        if(symbol_table.find(st)==symbol_table.end())
            {
                symbol_table[st]=line_off+z;
                current_definition_list[st]=line_off+z;
                module[st]=mod_count;
                symbol_usecount[st]=0;
            }
        else{
            multiple_symbols.push_back(st);
        }
    }
}

//###############################//

//##Read the Definitionlist##//

void read_deflist(){
    int def_count=read_integers();
    if (def_count==-1){
        return;
    }
    if(def_count>max_def_len){
        ch_offset-=to_string(def_count).length();
        print_error(4);
    }
    flag = true;
    for (int i=0;i<def_count; i++) {
        read_definitions();
    }
}

//###############################//

//##Read the Use list Symbols and Return it as strings##//

string read_list(){
    string str = read_symbol();
    return str;
}

//###############################//

//##Read the Uselist##//

void read_uselist(){
    int use_count = read_integers();
    if(use_count>max_def_len){
        ch_offset-=to_string(use_count).length();
        print_error(5);
    }
    current_uselist.push_back(to_string(use_count));
    for(int i = 0;i<use_count;i++){
        string str = read_list();
        current_uselist.push_back(str);
    }
    for (int i = 1;i<=stoi(current_uselist[0]);i++) {
        tmp_symbol[current_uselist[i]]=false;
    }
}

//###############################//

//##Read the instructions##//

void read_instructions(){
    string add_mode =get_tokens();
    if(add_mode.compare("A")!=0&&add_mode.compare("E")!=0&&add_mode.compare("I")!=0&&add_mode.compare("R")!=0){
        ch_offset-=add_mode.length();
        print_error(2);
    }
    int instr = read_integers();
    bool def_length = true;
    bool act_length = true;
    bool absolute_add = true;
    bool relative_add = true;
    string st;
    if(second_pass_flag) {
        if(instr>9999){
            instr=9999;
            cout << setfill('0') << setw(3) << mod_no << ": " << setfill('0') << setw(4) << instr;
            if(add_mode.compare("I")==0){
                cout << " Error: Illegal immediate value; treated as 999";
            }
            else{
                cout<<" Error: Illegal opcode; treated as 9999";
            }
        }
        else {
            if (add_mode.compare("R") == 0) {
                int temp_var=instr%1000;
                if(temp_var>tmp_no){
                    relative_add=false;
                    instr = instr/1000*1000+line_off;
                }
                else {
                    instr = instr+line_off;
                }
            }
            if (add_mode.compare("E") == 0) {
                int oprand = instr % 1000;
                if(oprand>=stoi(current_uselist[0])){
                    act_length=false;
                }
                else {
                    string str = current_uselist[oprand+1];
                    tmp_symbol[str]=true;
                    if (symbol_table.find(str) == symbol_table.end())
                    {
                        def_length = false;
                        st = str;
                    } else {
                        symbol_usecount[str] += 1;
                        instr =instr/1000 * 1000+ symbol_table[str];
                    }
                }
            }
            if(add_mode.compare("A")==0){
                int oprand = instr%1000;
                if (oprand>max_mem){
                    instr = instr-oprand;
                    absolute_add=false;
                }
            }
            cout << setfill('0') << setw(3) << mod_no << ": " << setfill('0') << setw(4) << instr;
            if (!def_length) {
                cout << " Error: " << st << " is not defined; zero used";
            }
            if(!act_length){
                cout << " Error: External address exceeds length of uselist; treated as immediate";
            }
            if(!absolute_add){
                cout<< " Error: Absolute address exceeds machine size; zero used";
            }
            if(!relative_add){
                cout<<" Error: Relative address exceeds module size; zero use";
            }
        }
        cout << endl;
        ++mod_no;
        current_uselist.clear();
    }
}

//###############################//

//##Read the instructionlist##//

void read_instlist(){
    int num_count = read_integers();
    tmp_no=num_count;
    if(line_off+num_count>max_mem){
        ch_offset-=to_string(num_count).length();
        print_error(6);
    }
    for(int i=0;i<num_count;i++){
        read_instructions();
    }
    if(second_pass_flag){
        for (auto const &K : tmp_symbol) {
            if (!K.second) {
                cout << "Warning: Module " << mod_count << ": " << K.first
                     << " appeared in the uselist but was not actually use"<<endl;
            }
        }
        tmp_symbol.clear();
    }
    line_off+=tmp_no;
}

//###############################//

//##Module Creation##//

void create_mod(){
    mod_count++;
    read_deflist();
    if(flag) {
        read_uselist();
        read_instlist();
        for(auto const &L:current_definition_list){
            if(L.second>line_off){
                symbol_table[L.first]=line_off-tmp_no;
                cout<<"Warning: Module "<<mod_count<<": "<<L.first<<" too big "<<L.second<<" (max="<<line_off-1<<") assume zero relative"<<endl;
            }
        }
        current_definition_list.clear();
    }
    else return;
}
//###############################//

//##First Pass##//

void first_pass() {
    input = fopen(file_path,"r");
    if (input == NULL) perror ("Error opening file");
    else {
        while (!feof(input)) {
            create_mod();
            flag = false;
        }
    }
    fclose(input);
}

//###############################//

//##Second Pass##//

void second_pass(){
    input = fopen(file_path,"r");
    second_pass_flag = true;
    line_off = 0;
    mod_count=0;
    cout<<"\nMemory Map"<<endl;
    if (input == NULL) perror ("Error opening file");
    else{
        while (!feof(input)){
        create_mod();
        flag = false;
        }
    }
    cout<<endl;
    for (auto const &K : symbol_usecount) {
        if(K.second==0){
            cout<<"Warning: Module "<< module[K.first] <<": " << K.first<<" was defined but never used"<<endl;
        }
    }

    fclose(input);
}

//###############################//

//##Main Function to return the result##//

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
            file_path = argv[i];
            first_pass();
            print_symbol_table();
            second_pass();
            reinitialize();
    }
    return 0;
}
