#include <iostream>
#include <vector>
#include <string>
#include <sstream>
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
    vector<pair<string, vector<string>>> cfg;
    vector<string> inputSequence;
    vector<string> reductionSequence;
    vector<string> actions;

    // Read CFG
    string line;
    while (getline(cin, line)) {
        if (line == ".INPUT") break;
        auto tokens = split(line);
        if (tokens.size() < 2) continue;
        string lhs = tokens[0];
        vector<string> rhs(tokens.begin() + 1, tokens.end());
        cfg.emplace_back(lhs, rhs);
    }

    // Read INPUT
    while (getline(cin, line)) {
        if (line == ".ACTIONS") break;
        auto tokens = split(line);
        inputSequence.insert(inputSequence.end(), tokens.begin(), tokens.end());
    }

    // Read ACTIONS
    while (getline(cin, line)) {
        if (line == ".END") break;
        actions.push_back(line);
    }

    // Perform actions
    for (const auto &action : actions) {
        if (action == "print") {
            for (const auto &symbol : reductionSequence) {
                cout << symbol << " ";
            }
            cout << ". ";
            for (const auto &symbol : inputSequence) {
                cout << symbol << " ";
            }
            cout << endl;
        } else if (action == "shift") {
            if (!inputSequence.empty()) {
                reductionSequence.push_back(inputSequence[0]);
                inputSequence.erase(inputSequence.begin());
            }
        } else if (action.find("reduce") == 0) {
            int ruleNumber = stoi(action.substr(7));
            if (ruleNumber < cfg.size()) {
                auto &rule = cfg[ruleNumber];
                auto &rhs = rule.second;
                if (rhs.size() == 1 && rhs[0] == ".EMPTY") {
                    reductionSequence.push_back(rule.first);
                } else if (reductionSequence.size() >= rhs.size()) {
                    bool match = true;
                    for (size_t i = 0; i < rhs.size(); ++i) {
                        if (reductionSequence[reductionSequence.size() - rhs.size() + i] != rhs[i]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        reductionSequence.erase(reductionSequence.end() - rhs.size(), reductionSequence.end());
                        reductionSequence.push_back(rule.first);
                    }
                }
            }
        }
    }

    return 0;
}
