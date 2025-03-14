#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <utility>

using namespace std;

vector<string> split(const string &s) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    // Data structures to store grammar and parsing information
    vector<pair<string, vector<string>>> cfg;
    vector<string> inputSequence;
    map<pair<int, string>, int> transitions;
    map<int, vector<pair<int, string>>> reductions;
    string startSymbol;

    // Parse input components
    string line;
    while(getline(cin, line)) {
        if(line == ".INPUT")
            break;
        auto tokens = split(line);
        if(tokens.size() < 2) continue;
        string lhs = tokens[0];
        vector<string> rhs(tokens.begin()+1, tokens.end());
        if(cfg.empty())
            startSymbol = lhs;
        cfg.emplace_back(lhs, rhs);
    }

    // Read input sequence
    while(getline(cin, line)) {
        if(line == ".TRANSITIONS")
            break;
        auto tokens = split(line);
        inputSequence.insert(inputSequence.end(), tokens.begin(), tokens.end());
    }

    // Read transitions
    while(getline(cin, line)){
        if(line == ".REDUCTIONS")
            break;
        auto tokens = split(line);
        if(tokens.size() == 3) {
            int fromState = stoi(tokens[0]);
            string symbol = tokens[1];
            int toState = stoi(tokens[2]);
            transitions[{fromState, symbol}] = toState;
        }
    }

    // Read reductions
    while(getline(cin, line)) {
        if(line == ".END")
            break;
        auto tokens = split(line);
        if(tokens.size() == 3) {
            int stateNumber = stoi(tokens[0]);
            int ruleNumber = stoi(tokens[1]);
            string tag = tokens[2];
            reductions[stateNumber].emplace_back(ruleNumber, tag);
        }
    }

    // Initialize parsing state
    vector<pair<int, string>> stack = { {0, ""} };
    int inputIndex = 0;
    int shiftCount = 0;

    // Print initial configuration
    cout << ".";
    for(const auto &token : inputSequence)
        cout << " " << token;
    cout << endl;

    // Helper function to print parse configuration
    auto printLine = [&](const vector<pair<int, string>> &stk, const vector<string> &remainder) {
        bool first = true;
        for(const auto &p : stk) {
            if(p.second.empty()) continue;
            if(first) {
                cout << p.second;
                first = false;
            }
            else {
                cout << " " << p.second;
            }
        }
        cout << " .";
        for(const auto &tok : remainder)
            cout << " " << tok;
        cout << endl;
    };

    // Main parsing loop
    while(inputIndex < inputSequence.size()) {
        int currentState = stack.back().first;
        string currentInput = inputSequence[inputIndex];
        bool reduced = false;

        // Check for possible reductions
        if (reductions.find(currentState) != reductions.end()) {
            for(const auto &reduction : reductions[currentState]) {
                int ruleNumber = reduction.first;
                string tag = reduction.second;
                auto &rule = cfg[ruleNumber];

                // Check if reduction is valid based on lookahead
                if ((tag == currentInput) || (tag == "EOF" && currentInput == "EOF")) {
                    int rhsSize = rule.second.size();
                    if (rhsSize == 1 && rule.second[0] == ".EMPTY")
                        rhsSize = 0;

                    if (stack.size() >= (size_t)rhsSize) {
                        // Remove RHS symbols
                        stack.erase(stack.end() - rhsSize, stack.end());
                        int newState = stack.back().first;
                        string lhs = rule.first;

                        // Check for valid transition to LHS
                        if (transitions.find({newState, lhs}) != transitions.end()) {
                            stack.emplace_back(transitions[{newState, lhs}], lhs);
                            reduced = true;
                            vector<string> rem(inputSequence.begin()+inputIndex, inputSequence.end());
                            printLine(stack, rem);
                            break;
                        }
                    }
                }
            }
        }

        // If no reduction, attempt shift
        if(!reduced) {
            auto it = transitions.find({currentState, currentInput});
            if(it == transitions.end()) {
                cerr << "ERROR at " << (shiftCount+1) << endl;
                return 1;
            }
            stack.emplace_back(it->second, currentInput);
            inputIndex++;
            shiftCount++;
            vector<string> rem(inputSequence.begin()+inputIndex, inputSequence.end());
            printLine(stack, rem);
        }
    }

    // Final reduction pass for acceptance
    bool finalReductionPossible = true;
    while(finalReductionPossible) {
        finalReductionPossible = false;
        int currentState = stack.back().first;
        
        if (reductions.find(currentState) != reductions.end()) {
            for(const auto &reduction : reductions[currentState]) {
                if (reduction.second == ".ACCEPT") {
                    cout << startSymbol << " ." << endl;
                    return 0;
                }
            }
        }
    }

    cerr << "ERROR at " << (shiftCount+1) << endl;
    return 1;
}
