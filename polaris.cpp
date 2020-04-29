// Includes
#include "polaris.hpp"

int main(int argc, char** argv)
{
    polaris_setup();
    string file_path = check_args(argc, argv);
    string base_path = join_paths(".", get_working_directory(file_path));
    eval(load_source_file(file_path), base_path);
    return 0;
}

// -- get_working_directory: given a file, returns the directory that file is
// -- in.
string get_working_directory(string base_path)
{
    for(size_t i = base_path.length() - 1; i > 0; i--)
    {
        if(base_path[i] == '/'){
            base_path = base_path.substr(0, i);
            break;
        }
    }
    return base_path;
}

// -- join_paths: given two paths, if the second path is relative it is anexed
// -- to the first path and returned. Otherwise it is expanded and returned.
string join_paths(string base_path, string new_path)
{
    string joined_path = "";
    if(new_path[0] == '/') //Absolute Path
            joined_path = new_path;
    else if(new_path[0] == '~' && new_path[1] == '/') //Home Path
        joined_path = expand_home(new_path);
    else // Relative path
        joined_path = base_path + "/" + new_path;
    return joined_path;
}

// -- polaris_exit: closes everything and exits
void polaris_exit(int code)
{
    sqlite3_close(DB);
}

// -- polaris_setup: sets up global configurations for polaris
void polaris_setup()
{
    srand(time(0) * clock());
    if (sqlite3_open(expand_home("~/.polaris.db").c_str(), &DB))
        error((string) "Error opening DB " + sqlite3_errmsg(DB));
}

// -- check_args: checks and executes command line flags and arguments
// -- if a filename is found, said filename is returned
string check_args(int argc, char** argv)
{
    string filename = "";
    if (argc > 1)
    {
        for (unsigned int i = 1; i < argc; ++i)
        {
            string argument = argv[i];
            if (argument == "-v") display_version();
            else if (argument == "-h") display_help();
            else if (argument == "-p") show_pushpops = true;
            else
            {
                filename = argument;
                if (i < argc - 1)
                {
                    warning("some switches after the filename have been ignored.");
                    //TODO these commands should be pushed to the stack.
                }
            }
        }
    }
    if (filename == "")
        error("\r\nUsage: polaris <file>\r\nRun polaris -h for more information");
    return filename;
}

// -- error: displays an error message and exits polaris
void error(string message)
{
    cout << "Polaris Error: " << message << endl;
    polaris_exit(1);
}

// -- warning: displays a warning message without exiting polaris
void warning(string message)
{
    cout << "Polaris Warning: " << message << endl;
}

// -- display_version: display the polaris version message, then exits
void display_version()
{
    cout << "This is Polaris for Unix version " << VERSION << endl;
    cout << "Copyright 2020, Martin del Rio (www.lartu.net)." << endl;
    polaris_exit(0);
}

// -- display_help: displays polaris help information, then exits
void display_help()
{
    puts("Usage:");
    puts("  Polaris <source file>|<switch>");
    puts("Switches:");
    puts("  -v              Display Polaris version information.");
    puts("  -h              Display this help.");
    puts("  -m              Display memory information.");
    puts("  -p              Show push and pops during execution.");
    puts("Complete documentation for Polaris should be found on this");
    puts("system using the 'man Polaris' command. If you have access");
    puts("to the internet, the documentation can also be found online");
    puts("at www.lartu.net/projects/Polaris.");
    polaris_exit(0);
}

// -- exec: executes a system command and returns its output as a string
string exec(const char* cmd)
{
    string result = "";
    FILE *fpipe;
    char c = 0;
    if (0 == (fpipe = (FILE*) popen(cmd, "r")))
    {
        perror("popen() failed.");
        polaris_exit(EXIT_FAILURE);
    }
    while (fread(&c, sizeof c, 1, fpipe))
        result += c;
    pclose(fpipe);
    return result;
}

// -- expand_home: takes a path and expands ~ as the home directory
// -- then the new expanded path is returned
string expand_home(string filename)
{
    string newPath = "";
    string homeDir = exec("echo $HOME");
    trim(homeDir);
    for(size_t i = 0; i < filename.length(); ++i)
        if(i == 0 && filename[i] == '~' && filename[i+1] == '/')
            newPath += homeDir;
        else
            newPath += filename[i];
    return newPath;
}

// -- load_source_file: takes the path to a file and returns its contents
string load_source_file(string filename)
{
    string contents = "";
    filename = expand_home(filename);
    ifstream file(filename.c_str());
    if(!file.is_open())
        error("the file '" + filename + "' couldn't be loaded.");
    string line = "";
    while(getline(file, line))
        contents += line + "\n";
    return contents;
}

// -- tokenize: takes a polaris script and splits it into tokens
// -- the returned vector pair has if the token has to be evaluated first
// -- at its first position and the actual token at the second one
vector<pair<bool, string> > tokenize(string & source){
    vector<pair<bool, string> > tokens;
    string current_token = "";
    bool open_comment = false;
    unsigned int open_block_strings = 0;
    bool open_quoted_string = false;
    // Cuando se cerraron todos los parentesis se pushea
    // Sino cuando se cerraron todos los "
    // Sino cuando hay un espacio
    for(size_t i = 0; i <= source.length(); ++i)
    {
        char current_char = source[i];
        char next_char = source[i + 1];
        if(current_char == '/' && next_char == '*'
        && !open_quoted_string)
        {
            open_comment = true;
            ++i;
        }
        else if(current_char == '*' && next_char == '/'
        && !open_quoted_string)
        {
            open_comment = false;
            ++i;
        }
        else if(current_char == '(' && !open_comment && !open_quoted_string)
        {
            open_block_strings++;
            if(open_block_strings > 1)
            {
                current_token += current_char;
            }
        }
        else if(current_char == ')' && !open_comment && !open_quoted_string)
        {
            open_block_strings--;
            if(open_block_strings > 0)
            {
                current_token += current_char;
            }
            else
            {
                tokens.push_back(make_pair(false, current_token));
                current_token = "";
            }
        }
        else if(current_char == '"' && !open_comment)
        {
            if(open_quoted_string)
            {
                if(open_block_strings > 0)
                {
                    current_token += '"';
                }
                else
                {
                    tokens.push_back(make_pair(false, current_token));
                    current_token = "";
                }
            }
            else
            {
                if(open_block_strings > 0)
                    current_token += '"';
            }
            open_quoted_string = !open_quoted_string;
        }
        else if(
        (current_char == ' '  || current_char == '\t' ||
        current_char == '\n' || current_char == '\0' )
        && !open_comment  && open_block_strings == 0
        && !open_quoted_string)
        {
            if(current_token.length() > 0)
            {
                tokens.push_back(make_pair(true, current_token));
                current_token = "";
            }
        }
        else if(!open_comment)
        {
            current_token += current_char;
        }
    }
    return tokens;
}

// -- stack_push: pushes a token (maybe trimmed and maybe empty) to the stack.
void stack_push(string token, bool trim_token, bool pushempty)
{
    if (trim_token) trim(token);
    if (token.length() > 0 || pushempty)
    {
        p_stack.push(token);
        if(show_pushpops) cout << "Push: '" << token << "'" << endl;
    }
}

// -- stack_pop: pops a value from the stack
string stack_pop()
{
    if(p_stack.size() == 0)
        error("cannot pop from an empty stack.");
    string val = p_stack.top();
    p_stack.pop();
    if(show_pushpops) cout << "Pop: '" << val << "'" << endl;
    return val;
}

// -- polaris_delay: pauses the execution for some milliseconds
void polaris_delay(int milliseconds)
{
    struct timespec  req, rem;
    if (milliseconds <= 0L)
        return;
    req.tv_sec = milliseconds / 1000L;
    req.tv_nsec = (milliseconds % 1000L) * 1000000L;
    rem.tv_sec = 0;
    rem.tv_nsec = 0;
    nanosleep(&req, &rem);
}

// -- str_to_num: converts a string into a number
pnumber str_to_num(string & token)
{
    return strtod(token.c_str(), NULL);
}

// -- num_to_str: takes a number and converts it to a string
string num_to_str(pnumber value)
{
    stringstream s;
    s << fixed << value;
    string str = s.str();
    trim(str);
    str.erase(str.find_last_not_of('0') + 1, string::npos);
    str.erase(str.find_last_not_of('.') + 1, string::npos);
    return str;
}

// -- str_is_num: returns true if the passed string represents a valid number
bool str_is_num(string & source){
    if (source == "inf") return true;
    if (source == "-inf") return true;
    bool already_found_sign = false;
    bool already_found_point = false;
    size_t numbers_before_point = 0;
    size_t numbers_after_point = 0;
    for(size_t i = 0; i < source.length(); ++i)
        if(!already_found_sign && source[i] == '-')
            already_found_sign = true;
        else if(!already_found_point && source[i] == '.')
            already_found_point = true;
        else if(
        source[i] == '0' || source[i] == '1' 
        || source[i] == '2' || source[i] == '3'
        || source[i] == '4' || source[i] == '5'
        || source[i] == '6' || source[i] == '7'
        || source[i] == '8' || source[i] == '9'
        )
        {
            already_found_sign = true;
            if(already_found_point)
                numbers_after_point++;
            else
                numbers_before_point++;
        }
        else
            return false;
    if(numbers_before_point == 0)
        return false;
    if(already_found_point && numbers_after_point == 0)
        return false;
    return true;
}

// -- set_var_value: sets the value of a string
void set_var_value(string & var, string value)
{
    variables[var] = value;
}

// -- get_var_value: gets the value of a string
void get_var_value(string & var)
{
    if (variables.find(var) == variables.end())
        stack_push("", false, true);
    else
        stack_push(variables[var], false, true);
}

// -- check_numerical_values: checks if a pair of values are numerical
void check_numerical_values(string & str1, string & str2){
    check_numerical_values(str1);
    check_numerical_values(str2);
}

// -- check_numerical_values: checks if a value is numerical
void check_numerical_values(string & str){
    if(!str_is_num(str))
    {
        error("trying to operate arithmetically with a non-numerical value.");
    }
}

// -- eval: takes a polaris script and executes it
void eval(string source, string base_path)
{
    vector<pair<bool, string> > tokens = tokenize(source);
    for(size_t i = 0; i < tokens.size(); ++i){
        if(tokens[i].first)
            execute_string(tokens[i].second, base_path);
        else
            stack_push(tokens[i].second, false, true);
    }
}

// -- execute_string: takes a plain polaris string and evaluates it
void execute_string(string & token, string & base_path)
{
    trim(token);
    if(token.length() == 0) return;
    /* print */
    if(token == "print")
    {
        string source = stack_pop();
        size_t val_len = source.length();
        for(size_t i = 0; i < val_len; ++i)
        {
            if(source[i] == '\\' && i < val_len && source[i+1] == 'n')
            {
                printf("\n");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == 'r')
            {
                printf("\r");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == 't')
            {
                printf("\t");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == 'b')
            {
                printf("\b");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == 'a')
            {
                printf("\a");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == 'v')
            {
                printf("\v");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == 'f')
            {
                printf("\f");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == '\\')
            {
                printf("\\");
                ++i;
            }
            else if(source[i] == '\\' && i < val_len && source[i+1] == '"')
            {
                printf("\"");
                ++i;
            }
            else
            {
                printf("%c", source[i]);
            }
        }
        fflush(stdout);
    }
    /* + */
    else if(token == "+")
    {
        string value2 = stack_pop();
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1, value2);
        result = str_to_num(value1) + str_to_num(value2);
        stack_push(num_to_str(result), true, false);
    }
    /* - */
    else if(token == "-")
    {
        string value2 = stack_pop();
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1, value2);
        result = str_to_num(value1) - str_to_num(value2);
        stack_push(num_to_str(result), true, false);
    }
    /* * */
    else if(token == "*")
    {
        string value2 = stack_pop();
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1, value2);
        result = str_to_num(value1) * str_to_num(value2);
        stack_push(num_to_str(result), true, false);
    }
    /* / */
    else if(token == "/")
    {
        string value2 = stack_pop();
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1, value2);
        result = str_to_num(value1) / str_to_num(value2);
        stack_push(num_to_str(result), true, false);
    }
    /* % */
    else if(token == "%")
    {
        string value2 = stack_pop();
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1, value2);
        result = (pnumber_i) str_to_num(value1) % (pnumber_i) str_to_num(value2);
        stack_push(num_to_str(result), true, false);
    }
    /* // */
    else if(token == "//")
    {
        string value2 = stack_pop();
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1, value2);
        result = (pnumber_i) (str_to_num(value1) / str_to_num(value2));
        stack_push(num_to_str(result), true, false);
    }
    /* ** */
    else if(token == "**")
    {
        string value2 = stack_pop();
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1, value2);
        result = pow(str_to_num(value1), str_to_num(value2));
        stack_push(num_to_str(result), true, false);
    }
    /* sin */
    else if(token == "sin")
    {
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1);
        result = sin(str_to_num(value1));
        stack_push(num_to_str(result), true, false);
    }
    /* cos */
    else if(token == "cos"){
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1);
        result = cos(str_to_num(value1));
        stack_push(num_to_str(result), true, false);
    }
    /* tan */
    else if(token == "tan"){
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1);
        result = tan(str_to_num(value1));
        stack_push(num_to_str(result), true, false);
    }
    /* log */
    else if(token == "log"){
        string value1 = stack_pop();
        pnumber result;
        check_numerical_values(value1);
        result = log(str_to_num(value1));
        stack_push(num_to_str(result), true, false);
    }
    /* = */
    else if(token == "="){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(str_is_num(value1) && str_is_num(value2))
        {
            pnumber val1 = str_to_num(value1);
            pnumber val2 = str_to_num(value2);
            stack_push(fabs(val1 - val2) < EPSILON ? "1" : "0", true, false);
        }
        else
        {
            if(value1 == value2)
                stack_push("1", true, false);
            else
                stack_push("0", true, false);
        }
    }
    /* != */
    else if(token == "!="){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(str_is_num(value1) && str_is_num(value2)){
            pnumber val1 = str_to_num(value1);
            pnumber val2 = str_to_num(value2);
            stack_push(fabs(val1 - val2) > EPSILON ? "1" : "0", true, false);
        }
        else
        {
            if(value1 != value2)
                stack_push("1", true, false);
            else
                stack_push("0", true, false);
        }
    }
    /* ! */
    else if(token == "!"){
        string value1 = stack_pop();
        if(value1 == "0")
            stack_push("1", true, false);
        else
            stack_push("0", true, false);
    }/* < */
    else if(token == "<"){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(str_is_num(value1) && str_is_num(value2)){
            pnumber val1 = str_to_num(value1);
            pnumber val2 = str_to_num(value2);
            stack_push(val1 < val2 ? "1" : "0", true, false);
        }else{
            if(value1 < value2)
                stack_push("1", true, false);
            else
                stack_push("0", true, false);
        }
    }
    /* > */
    else if(token == ">"){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(str_is_num(value1) && str_is_num(value2)){
            pnumber val1 = str_to_num(value1);
            pnumber val2 = str_to_num(value2);
            stack_push(val1 > val2 ? "1" : "0", true, false);
        }else{
            if(value1 > value2)
                stack_push("1", true, false);
            else
                stack_push("0", true, false);
        }
    }
    /* <= */
    else if(token == "<="){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(str_is_num(value1) && str_is_num(value2)){
            pnumber val1 = str_to_num(value1);
            pnumber val2 = str_to_num(value2);
            stack_push(val1 <= val2 ? "1" : "0", true, false);
        }else{
            if(value1 <= value2)
                stack_push("1", true, false);
            else
                stack_push("0", true, false);
        }
    }
    /* >= */
    else if(token == ">="){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(str_is_num(value1) && str_is_num(value2)){
            pnumber val1 = str_to_num(value1);
            pnumber val2 = str_to_num(value2);
            stack_push(val1 >= val2 ? "1" : "0", true, false);
        }else{
            if(value1 >= value2)
                stack_push("1", true, false);
            else
                stack_push("0", true, false);
        }
    }
    /* & */
    else if(token == "&"){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(value1 != "0" && value2 != "0")
            stack_push("1", true, false);
        else
            stack_push("0", true, false);
    }
    /* | */
    else if(token == "|"){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(value1 != "0" || value2 != "0")
            stack_push("1", true, false);
        else
            stack_push("0", true, false);
    }
    /* eval */
    else if(token == "eval"){
        eval(stack_pop(), base_path);
    }
    /* set */
    else if(token == "set"){
        string var = stack_pop();
        string value = stack_pop();
        set_var_value(var, value);
    }
    /* get */
    else if(token == "get"){
        string var = stack_pop();
        get_var_value(var);
    }
    /* if */
    else if(token == "if"){
        string else_block = stack_pop();
        string if_block = stack_pop();
        string condition_block = stack_pop();
        eval(condition_block, base_path);
        string result = stack_pop();
        if(result != "0")
            eval(if_block, base_path);
        else
            eval(else_block, base_path);
    }
    /* while */
    else if(token == "while"){
        string while_block = stack_pop();
        string condition_block = stack_pop();
        while(true){
            eval(condition_block, base_path);
            string result = stack_pop();
            if(result != "0")
                eval(while_block, base_path);
            else
                break;
        }
    }
    /* length */
    else if(token == "length"){
        string value = stack_pop();
        stack_push(num_to_str(value.length()), false, true);
    }
    /* slice */
    else if(token == "slice"){
        string value2 = stack_pop();
        string value1 = stack_pop();
        if(str_is_num(value1) && str_is_num(value2)){
            string value = stack_pop();
            size_t from = str_to_num(value1);
            size_t count = str_to_num(value2);
            if(count < 0) count = value.length();
            stack_push(value.substr(from, count), false, true);
        }
        else
            error("trying to slice a string with non-numeric values.");
    }
    /* trim */
    else if(token == "trim"){
        string value = stack_pop();
        trim(value);
        stack_push(value, false, true);
    }
    /* join */
    else if(token == "join"){
        string value2 = stack_pop();
        string value1 = stack_pop();
        stack_push(value1 + value2, false, true);
    }
    /* copy */
    else if(token == "copy"){
        string value = stack_pop();
        stack_push(value, false, true);
        stack_push(value, false, true);
    }
    /* del */
    else if(token == "del"){
        stack_pop();
    }
    /* swap */
    else if(token == "swap"){
        string value2 = stack_pop();
        string value1 = stack_pop();
        stack_push(value2, false, true);
        stack_push(value1, false, true);
    }
    /* input */
    else if(token == "input"){
        string input;
        cin >> input;
        stack_push(input, false, true);
    }
    /* >var */
    else if(token.length() > 1 && token[0] == '>'){
        string value = stack_pop();
        string var_name = token.substr(1);
        set_var_value(var_name, value);
    }
    /* @var */
    else if(token.length() > 1 && token[0] == '@'){
        string var_name = token.substr(1);
        get_var_value(var_name);
    }
    /* % */
    else if(token.length() > 1 && token[token.length() - 1] == '%'){
        string var_name = token.substr(0, token.length() - 1);
        get_var_value(var_name);
        eval(stack_pop(), base_path);
    }
    /* exec */
    else if(token == "exec"){
        stack_push(exec(stack_pop().c_str()), false, true);
    }
    /* random */
    else if(token == "random"){
        stack_push(num_to_str(((pnumber) rand() / (RAND_MAX))), true, false);
    }
    /* exit */
    else if(token == "exit")
    {
        polaris_exit(0);
    }
    /* import */
    else if(token == "import")
    {
        int i = 0;
        string value = stack_pop();
        string new_path = "";
        string file_contents;
        if(value.length() == 0)
        {
            error("Invalid route for import (empty string).");
        }
        new_path = join_paths(base_path, value);
        file_contents = load_source_file(new_path);
        new_path = get_working_directory(new_path);
        eval(file_contents, new_path);
    }
    /* sleep */
    else if(token == "sleep")
    {
        string value1 = stack_pop();
        if(!str_is_num(value1))
        {
            error("trying to sleep a non-numerical amount of time.");
        }
        polaris_delay(str_to_num(value1));
    }
    // Query
    else if(token == "query")
    {
        /*string query = stack_pop();
        int return_code = 0; 
        char* error_msg; 
        return_code = sqlite3_exec(DB, query.c_str(), NULL, 0, &error_msg);
        if (return_code != SQLITE_OK) { 
            warning(error_msg);
            sqlite3_free(error_msg); 
        }*/
        //TODO terminar esto
    }
    /* Number */
    else if(str_is_num(token)){
        pnumber value = str_to_num(token);
        stack_push(num_to_str(value), true, false);
    }
    /* Plain strings */
    else
    {
        stack_push(token, false, true);
    }
}


