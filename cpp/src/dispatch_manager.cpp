#include "dispatch_manager.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

DispatchManager::DispatchManager() : nextRequestId_(1001), nextArrivalOrder_(1) {
    drone_heap_init(&inbox_);
    drone_queue_init(&pending_);
    drone_graph_init_demo(&graph_);
    drone_id_map_init(&idIndex_);
    drone_history_init(&history_);

    // Fictional teams and locations for a repeatable classroom demonstration.
    teams_.push_back(std::make_unique<MedicalTeam>(11, "M-01", 3));
    teams_.push_back(std::make_unique<RescueTeam>(21, "R-01", 4));
    teams_.push_back(std::make_unique<SupplyTeam>(31, "S-01", 5));
}

DispatchManager::~DispatchManager() { drone_history_clear(&history_); }

int DispatchManager::zoneCount() const { return static_cast<int>(graph_.zone_count); }

std::string DispatchManager::zoneName(int zoneId) const {
    if (zoneId < 0 || static_cast<size_t>(zoneId) >= graph_.zone_count) return "Unknown zone";
    return graph_.names[zoneId];
}

int DispatchManager::submitReport(RequestType type, Severity severity, int zoneId,
                                  const std::string& source, const std::string& description) {
    if (zoneId < 0 || static_cast<size_t>(zoneId) >= graph_.zone_count ||
        source.empty() || description.empty() || requests_.size() >= DRONE_MAX_ITEMS ||
        drone_heap_size(&inbox_) >= DRONE_MAX_ITEMS)
        return -1;

    const int id = nextRequestId_++;
    const unsigned long order = nextArrivalOrder_++;
    auto request = std::make_unique<Request>(id, type, severity, zoneId, source, description, order);
    const int index = static_cast<int>(requests_.size());
    if (!drone_id_map_put(&idIndex_, id, index)) return -1;
    DronePriorityItem item{id, static_cast<int>(severity), order};
    if (!drone_heap_push(&inbox_, item)) return -1;
    requests_.push_back(std::move(request));

    std::ostringstream event;
    event << "Report #" << id << " received: " << toString(severity) << ' '
          << toString(type) << " at " << zoneName(zoneId) << " (source: " << source << ").";
    recordEvent(event.str());
    std::cout << "Recorded report #" << id << " at " << zoneName(zoneId)
              << ". It is in the intake heap until the operator processes the inbox.\n";
    return id;
}

void DispatchManager::processInbox() {
    if (drone_heap_size(&inbox_) == 0) {
        std::cout << "The intake inbox is empty.\n";
        return;
    }
    DronePriorityItem item{};
    std::cout << "Processing reports by severity (earlier report first on a tie):\n";
    while (drone_heap_pop(&inbox_, &item)) {
        Request* request = findRequest(item.request_id);
        if (request != nullptr && request->status() == RequestStatus::InInbox)
            dispatchOrWait(*request);
    }
}

void DispatchManager::dispatchOrWait(Request& request) {
    int distance = 0;
    VolunteerTeam* team = nearestAvailableTeam(request, &distance);
    if (team != nullptr) {
        request.assignTo(team->id());
        team->setAvailable(false);
        printAssignment(request, *team, distance);
        std::ostringstream event;
        event << "Report #" << request.id() << " assigned to " << team->name()
              << " (" << team->responseLabel() << ").";
        recordEvent(event.str());
        return;
    }
    request.setStatus(RequestStatus::Waiting);
    if (!drone_queue_enqueue(&pending_, request.id())) {
        request.setStatus(RequestStatus::InInbox);
        std::cout << "Waiting queue is full; report #" << request.id()
                  << " could not be added. Operator action is required.\n";
        recordEvent("Waiting queue full; operator action required for report #" + std::to_string(request.id()) + ".");
        return;
    }
    std::cout << "Report #" << request.id() << " is waiting: no available "
              << toString(request.type()) << " team.\n";
    recordEvent("Report #" + std::to_string(request.id()) + " placed in the pending queue.");
}

VolunteerTeam* DispatchManager::nearestAvailableTeam(const Request& request, int* outDistance) {
    VolunteerTeam* bestTeam = nullptr;
    int bestDistance = 2147483647;
    for (const auto& ownedTeam : teams_) {
        VolunteerTeam* team = ownedTeam.get();
        if (!team->isAvailable() || team->specialty() != request.type()) continue;
        int path[DRONE_MAX_ZONES];
        size_t length = DRONE_MAX_ZONES;
        int distance = 0;
        if (drone_graph_shortest_path(&graph_, team->zoneId(), request.zoneId(),
                                      path, &length, &distance) && distance < bestDistance) {
            bestTeam = team;
            bestDistance = distance;
        }
    }
    if (bestTeam != nullptr && outDistance != nullptr) *outDistance = bestDistance;
    return bestTeam;
}

void DispatchManager::printAssignment(const Request& request, const VolunteerTeam& team, int distance) const {
    int path[DRONE_MAX_ZONES];
    size_t length = DRONE_MAX_ZONES;
    int actualDistance = 0;
    std::cout << "Assigned report #" << request.id() << " (" << toString(request.severity())
              << ' ' << toString(request.type()) << ") to " << team.name() << " - "
              << team.responseLabel() << ".\n";
    std::cout << "Reason: specialty match; this team has the shortest path on the sample map.\n";
    if (drone_graph_shortest_path(&graph_, team.zoneId(), request.zoneId(),
                                  path, &length, &actualDistance)) {
        std::cout << "Suggested route: ";
        for (size_t i = 0; i < length; ++i) {
            if (i != 0) std::cout << " -> ";
            std::cout << zoneName(path[i]);
        }
        std::cout << " (" << distance << " demo distance units).\n";
    }
    std::cout << "Report source entered by operator: " << request.source() << ".\n";
}

void DispatchManager::resolveRequest(int requestId) {
    Request* request = findRequest(requestId);
    if (request == nullptr) { std::cout << "No report with that ID.\n"; return; }
    if (request->status() != RequestStatus::Assigned) {
        std::cout << "Only an assigned report can be resolved in this simulation. Current state: "
                  << toString(request->status()) << ".\n";
        return;
    }
    VolunteerTeam* team = findTeam(request->assignedTeamId());
    if (team != nullptr) team->setAvailable(true);
    request->setStatus(RequestStatus::Resolved);
    request->clearAssignment();
    std::cout << "Report #" << requestId << " marked resolved; the assigned team is available again.\n";
    recordEvent("Report #" + std::to_string(requestId) + " resolved; team released.");
    retryWaiting();
}

void DispatchManager::retryWaiting() {
    const size_t toCheck = drone_queue_size(&pending_);
    int requestId = 0;
    bool anyAssigned = false;
    for (size_t i = 0; i < toCheck && drone_queue_dequeue(&pending_, &requestId); ++i) {
        Request* request = findRequest(requestId);
        if (request == nullptr || request->status() != RequestStatus::Waiting) continue;
        int distance = 0;
        VolunteerTeam* team = nearestAvailableTeam(*request, &distance);
        if (team == nullptr) {
            (void)drone_queue_enqueue(&pending_, requestId);
            continue;
        }
        request->assignTo(team->id());
        team->setAvailable(false);
        printAssignment(*request, *team, distance);
        recordEvent("Waiting report #" + std::to_string(requestId) + " assigned to " + team->name() + ".");
        anyAssigned = true;
    }
    if (!anyAssigned && toCheck > 0) std::cout << "No waiting report matches a team released so far.\n";
}

Request* DispatchManager::findRequest(int requestId) {
    int index = -1;
    if (!drone_id_map_get(&idIndex_, requestId, &index) || index < 0 ||
        static_cast<size_t>(index) >= requests_.size()) return nullptr;
    return requests_[static_cast<size_t>(index)].get();
}

const Request* DispatchManager::findRequest(int requestId) const {
    int index = -1;
    if (!drone_id_map_get(&idIndex_, requestId, &index) || index < 0 ||
        static_cast<size_t>(index) >= requests_.size()) return nullptr;
    return requests_[static_cast<size_t>(index)].get();
}

VolunteerTeam* DispatchManager::findTeam(int teamId) {
    for (const auto& team : teams_) if (team->id() == teamId) return team.get();
    return nullptr;
}

void DispatchManager::printDashboard() const {
    size_t inboxCount = drone_heap_size(&inbox_);
    std::cout << "\n=== Offline dispatch dashboard ===\n"
              << "Reports: " << requests_.size() << " | Intake inbox: " << inboxCount
              << " | Waiting for a team: " << drone_queue_size(&pending_) << "\n";
    for (const auto& team : teams_)
        std::cout << team->name() << " | " << team->responseLabel() << " | at "
                  << zoneName(team->zoneId()) << " | "
                  << (team->isAvailable() ? "available" : "assigned") << "\n";
    std::cout << "Reminder: locations and roads are sample data; this is not live tracking.\n";
}

void DispatchManager::printZones() const {
    std::cout << "\n=== Sample zones (IDs are menu numbers + 1) ===\n";
    for (size_t i = 0; i < graph_.zone_count; ++i) std::cout << (i + 1) << ". " << graph_.names[i] << '\n';
    std::cout << "Road weights are fictional demo distance units.\n";
}

void DispatchManager::printRequestDetails(const Request& request) const {
    std::cout << "#" << request.id() << " | " << toString(request.severity()) << ' '
              << toString(request.type()) << " | " << zoneName(request.zoneId())
              << " | " << toString(request.status()) << " | source: " << request.source();
    if (request.assignedTeamId() >= 0) {
        for (const auto& team : teams_)
            if (team->id() == request.assignedTeamId()) std::cout << " | team: " << team->name();
    }
    std::cout << "\n  Report: " << request.description() << '\n';
}

void DispatchManager::printRequests() const {
    if (requests_.empty()) { std::cout << "No reports have been entered.\n"; return; }
    for (const auto& request : requests_) printRequestDetails(*request);
}

void DispatchManager::printWaiting() const {
    const size_t count = drone_queue_size(&pending_);
    if (count == 0) { std::cout << "No reports are waiting for a team.\n"; return; }
    std::cout << "Reports waiting for a suitable team (queue order):\n";
    for (size_t i = 0; i < count; ++i) {
        const size_t slot = (pending_.head + i) % DRONE_MAX_ITEMS;
        const Request* request = findRequest(pending_.request_ids[slot]);
        if (request != nullptr) printRequestDetails(*request);
    }
}

void DispatchManager::printHistory() const { drone_history_print(&history_); }

void DispatchManager::answerQuery(const std::string& query) const {
    std::string normalized = query;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::istringstream input(normalized);
    std::string word;
    input >> word;
    if (word == "request" || word == "report") {
        int id = -1;
        input >> id;
        const Request* request = findRequest(id);
        if (request != nullptr) printRequestDetails(*request);
        else std::cout << "No report found. Try a report ID such as 1001.\n";
    } else if (normalized.find("wait") != std::string::npos || normalized.find("pending") != std::string::npos) {
        printWaiting();
    } else if (normalized.find("available") != std::string::npos || normalized.find("team") != std::string::npos) {
        for (const auto& team : teams_)
            std::cout << team->name() << " (" << team->responseLabel() << ") at "
                      << zoneName(team->zoneId()) << ": "
                      << (team->isAvailable() ? "available" : "assigned") << '\n';
    } else if (normalized.find("priority") != std::string::npos || normalized.find("urgent") != std::string::npos) {
        std::cout << "The intake max-heap processes Critical, High, Medium, then Low reports."
                     " Earlier reports are processed first when severity is equal.\n";
    } else if (normalized.find("route") != std::string::npos || normalized.find("network") != std::string::npos) {
        std::cout << "Routes use Dijkstra on the predefined zone graph. They do not reflect live roads or hazards.\n";
    } else {
        std::cout << "Try: request 1001 | what is waiting? | available teams | how is priority decided? | route rules\n";
    }
}

void DispatchManager::recordEvent(const std::string& message) {
    (void)drone_history_append(&history_, message.c_str());
}
