#ifndef polarisp
#define polaris

// Constants
#define EPSILON 0.000001
#define VERSION "1.0"

// Includes
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <stack>
#include <map>
#include "cpptrim.h"
#include "math.h"
#include <sqlite3.h> 
using namespace std;

// Types
#define pnumber double
#define pnumber_i long

// Globals
bool show_pushpops = false;
stack<string> p_stack;
map<string, string> variables;
sqlite3* DB;

// Function declarations
string check_args(int argc, char** argv);
string get_working_directory(string base_path);
string join_paths(string base_path, string new_path);
void polaris_exit(int code);
void polaris_setup();
void error(string message);
void warning(string message);
void display_version();
void display_help();
string exec(const char* cmd);
string expand_home(string filename);
string load_source_file(string main_filename);
vector<pair<bool, string> > tokenize(string & source);
void stack_push(string token, bool trim_token, bool pushempty);
string stack_pop();
void polaris_delay(int milliseconds);
pnumber str_to_num(string & token);
string num_to_str(pnumber value);
bool str_is_num(string & source);
void set_var_value(string & var, string value);
void get_var_value(string & var);
void check_numerical_values(string & str1, string & str2);
void check_numerical_values(string & str);
void eval(string source, string base_path);
void execute_string(string & token, string & base_path);

#endif
