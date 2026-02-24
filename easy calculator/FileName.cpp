#define _USE_MATH_DEFINES
#include <iostream>
#include <cctype>
#include <limits>
#include <cmath>
#include <stack>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <iomanip>

using namespace std;

// 原有的提示函数
void usertodo() {
    cout << "Hello!!!!!!!!!!!!!!" << endl;
    cout << "This  calculator simulated with a computer is part of my attempt. " << endl;
    cout << "Firstly, you need to confirm the mode." << endl;
    cout << "It has different modes: " << endl;
    cout << "Entering R/r means you just need a regular calculator." << endl;
    cout << "Entering S/s means you need a scientific calculator." << endl;
    cout << "Entering U/u means you need a  calculator to unit transform." << endl;
    cout << "Entering P/p means you need a programmer calculator." << endl;
    cout << "Entering E/e means you need a expression calculator (support bracket & function & variable)." << endl;
    cout << "Please enter the corresponding letter to select your mode: " << endl;
    cout << "After entering, press Enter to confirm your choice." << endl;
    cout << "If you enter an invalid character, the program will prompt you to re-enter." << endl;
    cout << "Let's get started! ＞ ＜" << endl;
}

// 原有函数声明
void regularCalculator();
void scientificCalculator();
void unitTransformCalculator();
void programmerCalculator();
void expressionCalculator();

//核心数据结构与工具函数
typedef map<string, double> VariableMap;

// 1. 获取优先级
// 优先级规则（数字越大优先级越高）：
// - 括号 ()：0
// - 异或 ^：1（低于加减）
// - 加减 +-：2
// - 乘除取余 */%：3
// - 幂运算 **：4（高于乘除，右结合）
// - 内置函数（sqrt/sin等）：5
int getPriority(const string& token) {
    if (token == "sqrt" || token == "sin" || token == "cos" || token == "tan" || token == "ln" || token == "exp" || token == "abs") {
        return 5; // 内置函数优先级最高
    }
    if (token == "**") {
        return 4; // 幂运算优先级次之
    }
    if (token == "^") {
        return 1; // 异或优先级低于加减
    }
    if (token.size() == 1) {
        char op = token[0];
        if (op == '(' || op == ')') return 0;
        if (op == '+' || op == '-') return 2;
        if (op == '*' || op == '/' || op == '%') return 3;
    }
    return -2; // 非法
}

// 辅助：判断是否为内置函数
bool isFunction(const string& token) {
    static const vector<string> funcs = { "sqrt", "sin", "cos", "tan", "ln", "exp", "abs" };
    return find(funcs.begin(), funcs.end(), token) != funcs.end();
}

// 判断是否为合法变量名
bool isValidVariableName(const string& token) {
    if (token.empty()) return false;
    if (!isalpha(token[0])) return false; // 变量名必须以字母开头
    for (char c : token) {
        if (!isalpha(c) && !isdigit(c)) { // 只能包含字母/数字
            return false;
        }
    }
    return !isFunction(token); // 不能与函数名重复
}

// 判断是否为已定义变量
bool isVariable(const string& token, const VariableMap& varMap) {
    return varMap.find(token) != varMap.end();
}

// 2. 拆分表达式
vector<string> tokenize(const string& expr) {
    vector<string> tokens;
    string current; // 统一维护当前正在构建的字符串（数字/字母串）
    size_t n = expr.size();

    for (size_t i = 0; i < n; ++i) {
        char c = expr[i];
        if (isspace(c)) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        // 核心修改1：处理幂运算**（连续两个*）
        if (c == '*' && i + 1 < n && expr[i + 1] == '*') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back("**"); // 作为单个token
            i++; // 跳过下一个*，避免重复处理
            continue;
        }

        // 核心修改2：处理异或^
        if (c == '^') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(string(1, c));
            continue;
        }

        // 处理负号（区分一元负号和二元减号）
        if (c == '-') {
            bool isUnaryMinus = false;
            if (i == 0) { // 表达式开头
                isUnaryMinus = true;
            }
            else if (expr[i - 1] == '(') { // 左括号后
                isUnaryMinus = true;
            }
            else if (expr[i - 1] == '+' || expr[i - 1] == '-' || expr[i - 1] == '*' || expr[i - 1] == '/' ||
                expr[i - 1] == '%' || expr[i - 1] == '^' || expr[i - 1] == '*') { // 运算符后（包含^和*）
                isUnaryMinus = true;
            }

            if (isUnaryMinus) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                if (i + 1 < n && isdigit(expr[i + 1])) {
                    current += c; // 负号+数字，合并为负数
                }
                else {
                    tokens.push_back(string(1, c)); // 负号+变量/函数，独立token
                }
            }
            else {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                tokens.push_back(string(1, c));
            }
            continue;
        }

        // 数字或小数点：继续构建数字串
        if (isdigit(c) || c == '.') {
            current += c;
        }
        // 字母：继续构建字母串（函数/变量名）
        else if (isalpha(c)) {
            current += tolower(c); // 统一转为小写
        }
        // 其他运算符/括号/赋值符：处理已构建的current，再添加当前符号
        else if (c == '+' || c == '*' || c == '/' || c == '%' ||
            c == '(' || c == ')' || c == '=') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(string(1, c));
        }
        // 非法字符
        else {
            cout << "警告：检测到非法字符 '" << c << "'，已忽略" << endl;
            if (!current.empty()) {
                current.clear();
            }
        }
    }

    // 处理最后一个未完成的token
    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

// 3. 中缀转后缀
vector<string> infixToPostfix(const vector<string>& tokens, const VariableMap& varMap, bool& isValid) {
    vector<string> postfix;
    stack<string> opStack;
    isValid = true;

    for (const string& token : tokens) {
        if (token == "(") {
            opStack.push(token);
        }
        else if (token == ")") {
            // 弹出到左括号为止
            while (!opStack.empty() && opStack.top() != "(") {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            // 括号不匹配
            if (opStack.empty()) {
                cout << "错误：右括号多余，括号不匹配" << endl;
                isValid = false;
                return postfix;
            }
            opStack.pop(); // 弹出左括号

            // 函数名：括号匹配后弹出函数到后缀表达式
            if (!opStack.empty() && isFunction(opStack.top())) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
        }
        // 函数或运算符：按优先级入栈
        else if (isFunction(token) || (token != "**" && getPriority(token) > 0)) {
            // 普通运算符（非**）：左结合，优先级>=时弹出
            while (!opStack.empty() && getPriority(opStack.top()) >= getPriority(token)) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
        }
        // 核心修改：幂运算**是右结合，优先级>时才弹出（而非>=）
        else if (token == "**" && getPriority(token) > 0) {
            while (!opStack.empty() && getPriority(opStack.top()) > getPriority(token)) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
        }
        // 变量：校验是否已定义
        else if (isValidVariableName(token)) {
            if (!isVariable(token, varMap)) {
                cout << "错误：未定义变量 '" << token << "'" << endl;
                isValid = false;
                return postfix;
            }
            postfix.push_back(token);
        }
        // 数字（包含负数）：直接加入后缀表达式
        else {
            try {
                stod(token); // 尝试转换，失败则抛异常
                postfix.push_back(token);
            }
            catch (...) {
                cout << "错误：非法数字/字符 '" << token << "'" << endl;
                isValid = false;
                return postfix;
            }
        }
    }

    // 弹出剩余运算符
    while (!opStack.empty()) {
        if (opStack.top() == "(") {
            cout << "错误：左括号多余，括号不匹配" << endl;
            isValid = false;
            return postfix;
        }
        postfix.push_back(opStack.top());
        opStack.pop();
    }

    return postfix;
}

// 4. 计算后缀表达式(逆波兰表达式)
double calculatePostfix(const vector<string>& postfix, const VariableMap& varMap, bool& isValid) {
    stack<double> numStack;
    isValid = true;

    for (const string& token : postfix) {
        // 处理内置函数
        if (isFunction(token)) {
            if (numStack.empty()) {
                cout << "错误：函数 '" << token << "' 缺少参数" << endl;
                isValid = false;
                return 0;
            }
            double param = numStack.top();
            numStack.pop();
            double res = 0;

            if (token == "sqrt") {
                if (param < 0) {
                    cout << "错误：sqrt函数参数不能为负数（当前值：" << param << "）" << endl;
                    isValid = false;
                    return 0;
                }
                res = sqrt(param);
            }
            else if (token == "sin") {
                res = sin(param); // 弧度制
            }
            else if (token == "cos") {
                res = cos(param);
            }
            else if (token == "tan") {
                res = tan(param);
            }
            else if (token == "ln") {
                if (param <= 0) {
                    cout << "错误：ln函数参数必须大于0（当前值：" << param << "）" << endl;
                    isValid = false;
                    return 0;
                }
                res = log(param);
            }
            else if (token == "exp") {
                res = exp(param);
            }
            else if (token == "abs") {
                res = fabs(param);
            }
            numStack.push(res);
        }
        // 处理一元负号
        else if (token == "-" && numStack.size() >= 1) {
            double val = numStack.top();
            numStack.pop();
            numStack.push(-val);
        }
        // 核心修改1：处理幂运算**
        else if (token == "**") {
            if (numStack.size() < 2) {
                cout << "错误：幂运算**操作数不足" << endl;
                isValid = false;
                return 0;
            }
            double b = numStack.top(); numStack.pop(); // 指数
            double a = numStack.top(); numStack.pop(); // 底数
            // 幂运算：a^b，处理特殊情况
            if (a < 0 && fmod(b, 1) != 0) {
                cout << "警告：负数的非整数次幂结果为复数，当前仅返回实数部分（" << a << "**" << b << "）" << endl;
            }
            double res = pow(a, b);
            numStack.push(res);
        }
        // 核心修改2：处理异或^（仅对整数有效）
        else if (token == "^") {
            if (numStack.size() < 2) {
                cout << "错误：异或^操作数不足" << endl;
                isValid = false;
                return 0;
            }
            double b = numStack.top(); numStack.pop();
            double a = numStack.top(); numStack.pop();
            // 检查是否为整数
            if (fmod(a, 1) != 0 || fmod(b, 1) != 0) {
                cout << "错误：异或^仅支持整数运算（当前值：" << a << "^" << b << "）" << endl;
                isValid = false;
                return 0;
            }
            // 转换为整数（处理负数，按补码）
            int ia = static_cast<int>(a);
            int ib = static_cast<int>(b);
            int res = ia ^ ib; // 异或运算
            numStack.push(static_cast<double>(res));
        }
        // 处理普通二元运算符
        else if (token.size() == 1 && getPriority(token) > 0) {
            if (numStack.size() < 2) {
                cout << "错误：运算符 '" << token << "' 操作数不足" << endl;
                isValid = false;
                return 0;
            }
            double b = numStack.top(); numStack.pop();
            double a = numStack.top(); numStack.pop();
            double res = 0;

            switch (token[0]) {
            case '+': res = a + b; break;
            case '-': res = a - b; break;
            case '*': res = a * b; break;
            case '/':
                if (fabs(b) < 1e-6) {
                    cout << "错误：除数不能为0" << endl;
                    isValid = false;
                    return 0;
                }
                res = a / b;
                break;
            case '%':
                if (fabs(b) < 1e-9) {
                    cout << "错误：取余运算除数不能为0" << endl;
                    isValid = false;
                    return 0;
                }
                res = fmod(a, b);
                break;
            default:
                cout << "错误：未知运算符 '" << token << "'" << endl;
                isValid = false;
                return 0;
            }
            numStack.push(res);
        }
        // 处理变量：从varMap取值（包含内置的pi、e）
        else if (isVariable(token, varMap)) {
            numStack.push(varMap.at(token));
        }
        // 处理数字（包含负数）
        else {
            try {
                double num = stod(token);
                numStack.push(num);
            }
            catch (...) {
                cout << "错误：数字转换失败（" << token << "）" << endl;
                isValid = false;
                return 0;
            }
        }
    }

    // 最终栈中只能有一个结果
    if (numStack.size() != 1) {
        cout << "错误：表达式格式错误，操作数与运算符不匹配" << endl;
        isValid = false;
    }
    return numStack.empty() ? 0 : numStack.top();
}

// 5. 提前处理赋值语句
bool processAssignment(const vector<string>& tokens, VariableMap& varMap, size_t& assignPos) {
    // 找到赋值符=的位置
    assignPos = string::npos;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "=") {
            assignPos = i;
            break;
        }
    }
    if (assignPos == string::npos) return false; // 不是赋值语句

    // 赋值符左边必须是单个变量名
    if (assignPos == 0) {
        cout << "错误：赋值符左边不能为空" << endl;
        return false;
    }
    string varName = tokens[assignPos - 1];
    if (!isValidVariableName(varName)) {
        cout << "错误：非法变量名！规则：1.以字母开头 2.仅含字母/数字 3.不能与函数名（sqrt/sin/cos/tan/ln/exp/abs）重复" << endl;
        return false;
    }

    // 内置常量不允许被覆盖
    if (varName == "pi" || varName == "e") {
        cout << "错误：内置常量 " << varName << " 不允许被重新赋值！" << endl;
        return false;
    }

    // 赋值符右边必须有表达式
    if (assignPos >= tokens.size() - 1) {
        cout << "错误：赋值符右边不能为空" << endl;
        return false;
    }
    // 提取右边的表达式tokens
    vector<string> exprTokens(tokens.begin() + assignPos + 1, tokens.end());

    // 计算右边表达式的值
    bool isValidExpr = true;
    vector<string> postfix = infixToPostfix(exprTokens, varMap, isValidExpr);
    if (!isValidExpr) {
        cout << "错误：赋值语句右边表达式无效" << endl;
        return false;
    }

    bool isValidCalc = true;
    double varValue = calculatePostfix(postfix, varMap, isValidCalc);
    if (!isValidCalc) {
        cout << "错误：赋值语句右边表达式计算失败" << endl;
        return false;
    }

    // 存入变量映射表
    varMap[varName] = varValue;
    cout << " 变量赋值成功：" << varName << " = " << fixed << setprecision(4) << varValue << endl;
    return true;
}

// 表达式计算器函数
void expressionCalculator() {
    cout << "\n=== 表达式计算器模式 ===" << endl;
    cout << "支持功能：" << endl;
    cout << "  1. 基础运算：加减(+-)、乘除(*/)、取余(%)、幂运算(**)、异或(^)" << endl;
    cout << "  2. 负数支持：表达式开头(-5+3)、括号内((-5)*2)、运算符后(3*-4)" << endl;
    cout << "  3. 内置单参数函数：sqrt(x)、sin(x)、cos(x)、tan(x)、ln(x)、exp(x)、abs(x)" << endl;
    cout << "  4. 内置常量：pi≈3.1416、e≈2.7183（不可覆盖）" << endl;
    cout << "  5. 自定义变量：字母开头、仅含字母/数字、不与函数名/内置常量重复" << endl;
    cout << "  6. 多语句：用分号分隔（如：a=10; b=a**2; b^3）" << endl;
    cout << "注意：" << endl;
    cout << "  - 三角函数参数为弧度，异或(^)仅支持整数运算" << endl;
    cout << "  - 幂运算(**)是右结合（2**3**2=2^(3^2)=512）" << endl;
    cout << "  - 所有符号请使用英文，退出输入 quit 并回车" << endl;
    cout << "--------------------------------------------------------" << endl;

    // 初始化内置常量 pi 和 e
    VariableMap varMap;
    varMap["pi"] = M_PI;    // 圆周率
    varMap["e"] = M_E;      // 自然常数

    string input;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清空输入缓冲区
    while (true) {
        cout << "\n请输入表达式：";
        getline(cin, input);

        // 退出逻辑
        string lowerInput = input;
        transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
        if (lowerInput == "quit") break;

        // 分号分隔多语句
        vector<string> statements;
        stringstream ss(input);
        string stmt;
        while (getline(ss, stmt, ';')) {
            // 去除语句前后空格
            stmt.erase(0, stmt.find_first_not_of(" \t"));
            stmt.erase(stmt.find_last_not_of(" \t") + 1);
            if (!stmt.empty()) statements.push_back(stmt);
        }

        // 逐个处理语句
        for (const string& s : statements) {
            vector<string> tokens = tokenize(s);
            if (tokens.empty()) continue;

            // 优先处理赋值语句
            size_t assignPos;
            bool isAssignment = processAssignment(tokens, varMap, assignPos);
            if (isAssignment) continue;

            // 普通表达式计算
            bool isValidExpr = true;
            vector<string> postfix = infixToPostfix(tokens, varMap, isValidExpr);
            if (!isValidExpr) {
                continue;
            }

            bool isValidCalc = true;
            double result = calculatePostfix(postfix, varMap, isValidCalc);
            if (!isValidCalc) {
                continue;
            }

            // 输出结果
            cout << "--------------------------------------------------------" << endl;
            cout << fixed << setprecision(4) << "运算结果：" << s << " = " << result << endl;
            cout << "--------------------------------------------------------" << endl;
        }
    }

    cout << "\n表达式计算器模式退出" << endl;
}

void regularCalculator() {
    cout << "\n=== 普通计算器模式 ===" << endl;
    cout << "该模式已实现加减乘除求余运算" << endl;
}

void scientificCalculator() {
    cout << "\n=== 科学计算器模式 ===" << endl;
    cout << "该模式已实现平方根、三角函数等运算" << endl;
}

void unitTransformCalculator() {
    cout << "\n=== 单位转换计算器模式 ===" << endl;
    cout << "该模式后续可实现：长度、重量、温度、时间等单位之间的转换" << endl;
}

void programmerCalculator() {
    cout << "\n=== 程序员计算器模式 ===" << endl;
    cout << "该模式后续可实现：进制转换、位运算等" << endl;
}
int main() {
    usertodo();
    char choice;
    bool validInput = false;
    while (!validInput) {
        cin >> choice;
        choice = tolower(choice);
        switch (choice) {
        case 'r': regularCalculator(); validInput = true; break;
        case 's': scientificCalculator(); validInput = true; break;
        case 'u': unitTransformCalculator(); validInput = true; break;
        case 'p': programmerCalculator(); validInput = true; break;
        case 'e': expressionCalculator(); validInput = true; break;
        default:
            cout << "Invalid input! Please enter R/r, S/s, U/u, P/p or E/e only: " << endl;
            break;
        }
    }
    return 0;
}