#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
using namespace std;

struct ParseTreeNode {
    string value;
    vector<ParseTreeNode*> children;
    ParseTreeNode(const string &val) : value(val) {}
    ~ParseTreeNode() {
        for (auto c : children) delete c;
    }
};

vector<pair<string,string>> tokens;
size_t tokenIndex = 0;
map<string,vector<vector<string>>> grammar;

void formatError(int tcount) {
    cerr << "ERROR at " << (tcount + 1) << endl;
    exit(1);
}

ParseTreeNode* match(const string &expected) {
    if (tokenIndex >= tokens.size() || tokens[tokenIndex].first != expected) {
        formatError(tokenIndex);
    }
    ParseTreeNode* leaf = new ParseTreeNode(tokens[tokenIndex].first + " " + tokens[tokenIndex].second);
    tokenIndex++;
    return leaf;
}

ParseTreeNode* parse(const string &nonterm);

ParseTreeNode* parse(const string &nonterm) {
    if (!grammar.count(nonterm)) {
        formatError(tokenIndex);
    }
    const auto &prods = grammar[nonterm];
    for (auto &prod : prods) {
        size_t savedIndex = tokenIndex;
        ParseTreeNode* node = new ParseTreeNode(nonterm);
        bool ok = true;
        for (auto &sym : prod) {
            if (sym == ".EMPTY") {
                node->value += " .EMPTY";
                return node;
            }
            if (grammar.count(sym)) {
                ParseTreeNode* child = parse(sym);
                if (!child) {
                    ok = false;
                    break;
                }
                node->children.push_back(child);
            } else {
                if (tokenIndex >= tokens.size() || tokens[tokenIndex].first != sym) {
                    ok = false;
                    break;
                }
                node->children.push_back(match(sym));
            }
        }
        if (ok) {
            {
                ostringstream line;
                line << nonterm;
                for (auto &s : prod) {
                    line << " " << s;
                }
                node->value = line.str();
            }
            return node;
        }
        delete node;
        tokenIndex = savedIndex;
    }
    return nullptr;
}

void printParseTree(ParseTreeNode* root) {
    if (!root) return;
    cout << root->value << endl;
    for (auto c : root->children) {
        printParseTree(c);
    }
}

void loadGrammar() {
    grammar["start"] = {{"BOF","procedures","EOF"}};
    grammar["procedures"] = {{"procedure","procedures"},{"main"}};
    grammar["procedure"] = {{
      "INT","ID","LPAREN","params","RPAREN","LBRACE","dcls","statements","RETURN","expr","SEMI","RBRACE"
    }};
    grammar["main"] = {{
      "INT","WAIN","LPAREN","dcl","COMMA","dcl","RPAREN","LBRACE","dcls","statements","RETURN","expr","SEMI","RBRACE"
    }};
    grammar["params"] = {{".EMPTY"},{"paramlist"}};
    grammar["paramlist"] = {{"dcl"},{"dcl","COMMA","paramlist"}};
    grammar["type"] = {{"INT"},{"INT","STAR"}};
    grammar["dcls"] = {
        {".EMPTY"},
        {"dcls","dcl","BECOMES","NUM","SEMI"},
        {"dcls","dcl","BECOMES","NULL","SEMI"}
    };
    grammar["dcl"] = {{"type","ID"}};
    grammar["statements"] = {{".EMPTY"},{"statements","statement"}};
    grammar["statement"] = {
        {"lvalue","BECOMES","expr","SEMI"},
        {"IF","LPAREN","test","RPAREN","LBRACE","statements","RBRACE","ELSE","LBRACE","statements","RBRACE"},
        {"WHILE","LPAREN","test","RPAREN","LBRACE","statements","RBRACE"},
        {"PRINTLN","LPAREN","expr","RPAREN","SEMI"},
        {"PUTCHAR","LPAREN","expr","RPAREN","SEMI"},
        {"DELETE","LBRACK","RBRACK","expr","SEMI"}
    };
    grammar["test"] = {
        {"expr","EQ","expr"},
        {"expr","NE","expr"},
        {"expr","LT","expr"},
        {"expr","LE","expr"},
        {"expr","GE","expr"},
        {"expr","GT","expr"}
    };
    grammar["expr"] = {
        {"term"},
        {"expr","PLUS","term"},
        {"expr","MINUS","term"}
    };
    grammar["term"] = {
        {"factor"},
        {"term","STAR","factor"},
        {"term","SLASH","factor"},
        {"term","PCT","factor"}
    };
    grammar["factor"] = {
        {"ID"},{"NUM"},{"NULL"},
        {"LPAREN","expr","RPAREN"},
        {"AMP","lvalue"},
        {"STAR","factor"},
        {"NEW","INT","LBRACK","expr","RBRACK"},
        {"ID","LPAREN","RPAREN"},
        {"ID","LPAREN","arglist","RPAREN"},
        {"GETCHAR","LPAREN","RPAREN"}
    };
    grammar["arglist"] = {
        {"expr"},{"expr","COMMA","arglist"}
    };
    grammar["lvalue"] = {
        {"ID"},{"STAR","factor"},{"LPAREN","lvalue","RPAREN"}
    };
}

int main() {
    loadGrammar();
    string line;
    while(true) {
        if(!getline(cin,line)) break;
        if(line.size() < 2) break;
        istringstream iss(line);
        string k;
        iss >> k;
        string lx;
        getline(iss,lx);
        if(!lx.empty() && lx[0]==' ') lx.erase(0,1);
        tokens.push_back({k,lx});
    }
    tokens.insert(tokens.begin(),{"BOF","BOF"});
    tokens.push_back({"EOF","EOF"});
    ParseTreeNode* root = parse("start");
    if(!root) {
        formatError(tokenIndex);
    }
    printParseTree(root);
    delete root;
    return 0;
}
