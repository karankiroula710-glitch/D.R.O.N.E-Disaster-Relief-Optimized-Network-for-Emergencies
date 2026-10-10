#include "dispatch_manager.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {
bool readNumber(const std::string& prompt, int minimum, int maximum, int& out) {
    std::string line;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) return false;
        std::istringstream input(line);
        int value = 0;
        char extra = '\0';
        if ((input >> value) && !(input >> extra) && value >= minimum && value <= maximum) {
            out = value;
            return true;
        }
        std::cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}

bool readText(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    if (!std::getline(std::cin, out)) return false;
    if (out.empty()) {
        std::cout << "This field cannot be blank.\n";
        return false;
    }
    return true;
}

bool intakeReport(DispatchManager& manager) {
    int typeChoice = 0, severityChoice = 0, zoneChoice = 0, sourceChoice = 0;
    if (!readNumber("Type: 1 Medical, 2 Rescue, 3 Supply: ", 1, 3, typeChoice)) return false;
    if (!readNumber("Severity: 1 Low, 2 Medium, 3 High, 4 Critical: ", 1, 4, severityChoice)) return false;
    manager.printZones();
    if (!readNumber("Reported zone number: ", 1, manager.zoneCount(), zoneChoice)) return false;
    std::cout << "Report source: 1 shelter desk, 2 checkpoint, 3 runner, 4 radio relay entered by operator.\n";
    if (!readNumber("Source: ", 1, 4, sourceChoice)) return false;
    std::string source;
    switch (sourceChoice) {
        case 1: source = "shelter desk"; break;
        case 2: source = "checkpoint"; break;
        case 3: source = "runner relay"; break;
        case 4: source = "radio relay (operator entered)"; break;
    }
    std::string description;
    if (!readText("Short fictional report (no real names or contact details): ", description)) return false;
    RequestType type = static_cast<RequestType>(typeChoice);
    Severity severity = static_cast<Severity>(severityChoice);
    return manager.submitReport(type, severity, zoneChoice - 1, source, description) >= 0;
}
}

int main() {
    DispatchManager manager;
    std::cout << "D.R.O.N.E. - Phase 2 offline dispatch simulation\n"
              << "This local console does not track people or contact emergency services.\n"
              << "Enter only fictional demonstration reports.\n";
    while (true) {
        std::cout << "\n1 Dashboard\n2 Enter a relayed report\n3 Process intake by priority\n"
                     "4 Show reports and waiting list\n5 Resolve assigned report\n6 Ask a local query\n"
                     "7 Dispatch history\n8 Show sample zones\n0 Exit\n";
        int choice = 0;
        if (!readNumber("Choose: ", 0, 8, choice)) break;
        if (choice == 0) break;
        switch (choice) {
            case 1: manager.printDashboard(); break;
            case 2: (void)intakeReport(manager); break;
            case 3: manager.processInbox(); break;
            case 4: manager.printRequests(); manager.printWaiting(); break;
            case 5: {
                int id = 0;
                if (readNumber("Assigned report ID to resolve: ", 1, 999999, id)) manager.resolveRequest(id);
                break;
            }
            case 6: {
                std::string query;
                if (readText("Query: ", query)) manager.answerQuery(query);
                break;
            }
            case 7: manager.printHistory(); break;
            case 8: manager.printZones(); break;
        }
    }
    std::cout << "Session ended. In-memory reports and history will be cleared.\n";
    return 0;
}
