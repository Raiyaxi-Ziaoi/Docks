// Macros

#define VERIFY(a, b, c) (args.size() < a || (b != -1 && args.size() > b) || (c != -1 && (args.size() < c || args.size() > c))) ? (std::cout << "Incorrect argument list size!" << std::endl, void()) : void()

#define LAMBDA(a) a, [](const std::vector<std::string>& args)
#define VARARG(a) ((args[a].find("$") != std::string::npos) ? var_buffer[args[a].substr(1)] : args[a])

#define VERIFY_LEAST(a) if (args.size() < a) { std::cout << "At least " << a << " arguments; " << args.size() << " arguments given" << std::endl; return; }
#define VERIFY_LOWHI(min, max) VERIFY(min, max, -1)
#define VERIFY_EXACT(n) if (args.size() != n) { std::cout << "Exactly " << n << " arguments requested; " << args.size() << " arguments given" << std::endl; return; }

// Imports

#include <unordered_map>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>

// Platform specific imports

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

// Buffers

static std::unordered_map<std::string, std::string> var_buffer{};
static std::unordered_map<std::string, std::string> fun_buffer{};

// Helper functions

std::string replace(std::string original, std::string to_replace, std::string replacement) {
    size_t pos = original.find(to_replace);
    if (pos != std::string::npos) {
        original.replace(pos, to_replace.length(), replacement);
    }
    return original;
}

double power(double base, int exponent) {
    double result = 1.0;

    if (exponent < 0) {
        base = 1.0 / base;
        exponent = -exponent;
    }
    
    while (exponent > 0) {
        if (exponent % 2 == 1) result *= base;
        base *= base;
        exponent /= 2;
    }

    return result;
}

std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) tokens.push_back(token);
    return tokens;
}

float modulo(float x, float y) { return x - (x / y) * y; }


void evaluate(std::string input);

// Boats

std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> boats
{
    {LAMBDA("put") // Print function: put text...
    {
        for (const auto& x : args) 
        {
            if (x.find("$") != std::string::npos) std::cout << var_buffer[x.substr(1)] << " ";
            else std::cout << replace(x, "\\n", "\n") << " ";
        }
        std::cout << std::endl;
    }},
    {LAMBDA("ext") // Exit function: ext
    {  
        exit(0);
    }},
    {LAMBDA("hlt") // Halt function: hlt millisecond_duration
    {
        VERIFY_EXACT(1);
        std::chrono::milliseconds hlt(std::stoi(args[0]));
        std::this_thread::sleep_for(hlt);
    }},
    {LAMBDA("var") // Variable function: var name value
    {
        VERIFY_EXACT(2);
        var_buffer[args[0]] = args[1];
    }},
    {LAMBDA("add") // Addition function: add result_variable left_value right_value
    {
        VERIFY_EXACT(3);
        auto x = VARARG(1); auto y = VARARG(2);
        var_buffer[args[0]] = std::to_string(std::stof(x) + std::stof(y));
    }},
    {LAMBDA("sub") // Subtraction function: sub result_variable left_value right_value
    {
        VERIFY_EXACT(3);
        auto x = VARARG(1); auto y = VARARG(2);
        var_buffer[args[0]] = std::to_string(std::stof(x) - std::stof(y));
    }},
    {LAMBDA("mul") // Multiplication function: mul result_variable left_value right_value
    {
        VERIFY_EXACT(3);
        auto x = VARARG(1); auto y = VARARG(2);
        var_buffer[args[0]] = std::to_string(std::stof(x) * std::stof(y));
    }},
    {LAMBDA("div") // Division function: div result_variable left_value right_value
    {
        VERIFY_EXACT(3);
        auto x = VARARG(1); auto y = VARARG(2);
        var_buffer[args[0]] = std::to_string(std::stof(x) / std::stof(y));
    }},
    {LAMBDA("mod") // Modulo function: mod result_variable left_value right_value
    {
        VERIFY_EXACT(3);
        auto x = VARARG(1); auto y = VARARG(2);
        var_buffer[args[0]] = std::to_string(modulo(std::stof(x), std::stof(y)));
    }},
    {LAMBDA("exp") // Exponentiation function: exp result_variable left_value right_value
    {
        VERIFY_EXACT(3);
        auto x = VARARG(1); auto y = VARARG(2);
        var_buffer[args[0]] = std::to_string(power(std::stof(x), std::stof(y)));
    }},
    {LAMBDA("cls") // Clear console function: cls
    {
        #ifdef _WIN32
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coord = {0, 0};
        DWORD count;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hStdOut, &csbi);
        DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
        FillConsoleOutputCharacter(hStdOut, ' ', cells, coord, &count);
        SetConsoleCursorPosition(hStdOut, coord);
        #else
        system("clear");
        #endif
    }},
    {LAMBDA("fun") // Function function: fun name function
    {
        VERIFY_LEAST(2);
        auto tmp = args; tmp.erase(tmp.begin());
        for (int i = 0; i < tmp.size(); i++) 
        {
            fun_buffer[args[0]] += tmp[i] + " ";
            fun_buffer[args[0]] = replace(fun_buffer[args[0]], ";", "\n"); 
        }
    }},
    {LAMBDA("ask") // User input: ask variable_name
    {
        VERIFY_EXACT(1);
        std::string input; std::getline(std::cin, input);
        var_buffer[args[0]] = input;
    }},
    {LAMBDA("run") // Run function: run function_name
    {
        VERIFY_EXACT(1);
        for (const auto& x : split_string(fun_buffer[args[0]], '\n')) evaluate(x);
    }},
    {LAMBDA("chk")
    {
                if (args[0].compare("-equ") == 0) 
            if (VARARG(1).compare(VARARG(2)) == 0) 
                for (const auto& x : split_string(fun_buffer[args[3]], '\n')) 
                    evaluate(x);
        else if (args[0].compare("-neq") == 0)
            if (VARARG(1).compare(VARARG(2)) != 0) 
                for (const auto& x : split_string(fun_buffer[args[3]], '\n')) 
                    evaluate(x); 
        else if (args[0].compare("-gre") == 0)
            if (std::stof(VARARG(1)) > std::stof(VARARG(2))) 
                for (const auto& x : split_string(fun_buffer[args[3]], '\n')) 
                    evaluate(x);
        else if (args[0].compare("-les") == 0)
            if (std::stof(VARARG(1)) < std::stof(VARARG(2))) 
                for (const auto& x : split_string(fun_buffer[args[3]], '\n')) 
                    evaluate(x);
        else if (args[0].compare("-geq") == 0)
            if (std::stof(VARARG(1)) >= std::stof(VARARG(2))) 
                for (const auto& x : split_string(fun_buffer[args[3]], '\n')) 
                    evaluate(x); 
        else if (args[0].compare("-leq") == 0)
            if (std::stof(VARARG(1)) <= std::stof(VARARG(2))) 
                for (const auto& x : split_string(fun_buffer[args[3]], '\n')) 
                    evaluate(x);
    }},
    {LAMBDA("use") // Import function: use module_name
    {
        VERIFY_EXACT(1);
        std::string line; std::ifstream infile(args[0]);
    
        while (std::getline(infile, line)) evaluate(line);
    }},
    {LAMBDA("del") // Delete function: del
    {
        if (args[0].compare("-var_spec") == 0) var_buffer.erase(VARARG(1));
        else if (args[0].compare("-var_all") == 0) var_buffer.clear();  
        else if (args[0].compare("-fun_spec") == 0) fun_buffer.erase(VARARG(1));
        else if (args[0].compare("-fun_all") == 0) fun_buffer.clear();
    }},
    {LAMBDA("rem") // Comments
    { }},
    {LAMBDA("nop") // No operation
    { }},
};

void evaluate(std::string input)
{
    std::vector<std::string> tkns;
    std::string tmp;
    std::istringstream extrct(input);
        
    while (extrct >> tmp) tkns.push_back(tmp);
    if (tkns.empty()) return;
        
    if (auto cmd = boats.find(tkns[0]); cmd != boats.end()) cmd->second(std::vector<std::string>(tkns.begin() + 1, tkns.end()));
    else std::cout << "Command is invalid: " << tkns[0] << std::endl;
}

int main() 
{    
    for (std::string input; std::cout << ">> " && std::getline(std::cin, input);) evaluate(input); 

    return 0;
}
