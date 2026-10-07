#include "dispatch_manager.hpp"

#include <iostream>

int main() {
    drone::DispatchManager manager;
    manager.loadDemonstrationScenario();
    manager.printDashboard();
    manager.printHistory();

    std::cout << "\n--- Resolving rescue request #1046 to demonstrate queue retry ---\n";
    if (!manager.resolveRequest(1046)) {
        std::cerr << "Could not resolve demonstration request #1046.\n";
        return 1;
    }
    manager.printDashboard();
    manager.printHistory();
    return 0;
}
