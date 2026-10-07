#include "dispatch_manager.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace drone {
namespace {

int zoneIndex(RequestType type) { return static_cast<int>(type); }

std::string formatDistance(double distanceKm) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << distanceKm << " km";
    return out.str();
}

} // namespace

Zone::Zone(int id, std::string name) : id_(id), name_(std::move(name)) {}
int Zone::id() const { return id_; }
const std::string& Zone::name() const { return name_; }

Request::Request(int id, RequestType type, Severity severity, int zoneId,
                 std::string description, unsigned long long receivedOrder)
    : id_(id), type_(type), severity_(severity), zoneId_(zoneId),
      description_(std::move(description)), receivedOrder_(receivedOrder),
      status_(RequestStatus::Queued) {}

int Request::id() const { return id_; }
RequestType Request::type() const { return type_; }
Severity Request::severity() const { return severity_; }
int Request::zoneId() const { return zoneId_; }
const std::string& Request::description() const { return description_; }
RequestStatus Request::status() const { return status_; }
const std::string& Request::assignedTeamId() const { return assignedTeamId_; }

VolunteerTeam::VolunteerTeam(std::string id, std::string name, RequestType type, int zoneId)
    : id_(std::move(id)), name_(std::move(name)), type_(type), zoneId_(zoneId),
      available_(true), assignedRequestId_(-1) {}

const std::string& VolunteerTeam::id() const { return id_; }
const std::string& VolunteerTeam::name() const { return name_; }
RequestType VolunteerTeam::type() const { return type_; }
int VolunteerTeam::zoneId() const { return zoneId_; }
bool VolunteerTeam::available() const { return available_; }
int VolunteerTeam::assignedRequestId() const { return assignedRequestId_; }

void VolunteerTeam::assign(int requestId, int destinationZone) {
    available_ = false;
    assignedRequestId_ = requestId;
    zoneId_ = destinationZone;
}

void VolunteerTeam::release() {
    available_ = true;
    assignedRequestId_ = -1;
}

MedicalTeam::MedicalTeam(std::string id, std::string name, int zoneId)
    : VolunteerTeam(std::move(id), std::move(name), RequestType::Medical, zoneId) {}
std::string MedicalTeam::responseDescription() const { return "medical aid and first response"; }

RescueTeam::RescueTeam(std::string id, std::string name, int zoneId)
    : VolunteerTeam(std::move(id), std::move(name), RequestType::Rescue, zoneId) {}
std::string RescueTeam::responseDescription() const { return "search, evacuation, and rescue"; }

SupplyTeam::SupplyTeam(std::string id, std::string name, int zoneId)
    : VolunteerTeam(std::move(id), std::move(name), RequestType::Supply, zoneId) {}
std::string SupplyTeam::responseDescription() const { return "essential relief supplies"; }

DispatchManager::DispatchManager()
    : historyHead_(nullptr), historyTail_(nullptr), nextReceivedOrder_(1) {
    drone_graph_init(&graph_, DRONE_MAX_VERTICES);
    drone_priority_queue_init(&priorityQueue_);
    drone_request_queue_init(&pendingQueue_);
    drone_request_index_init(&requestIndex_);
}

DispatchManager::~DispatchManager() {
    drone_history_clear(&historyHead_, &historyTail_);
}

bool DispatchManager::addZone(Zone zone) {
    if (zone.id() != static_cast<int>(zones_.size()) ||
        zones_.size() >= DRONE_MAX_VERTICES || zone.name().empty()) return false;
    zones_.push_back(std::move(zone));
    return true;
}

bool DispatchManager::connectZones(int fromZone, int toZone, double distanceKm) {
    if (findZone(fromZone) == nullptr || findZone(toZone) == nullptr) return false;
    return drone_graph_add_edge(&graph_, fromZone, toZone, distanceKm) != 0;
}

void DispatchManager::addTeam(std::unique_ptr<VolunteerTeam> team) {
    if (!team || team->id().empty() || team->name().empty() || findZone(team->zoneId()) == nullptr) return;
    const auto duplicate = std::find_if(teams_.begin(), teams_.end(), [&](const auto& existing) {
        return existing->id() == team->id();
    });
    if (duplicate != teams_.end()) return;
    teams_.push_back(std::move(team));
}

bool DispatchManager::submitRequest(Request request) {
    const int id = request.id();
    if (id < 0 || findZone(request.zoneId()) == nullptr || requests_.size() >= DRONE_PRIORITY_CAPACITY ||
        findRequest(id) != nullptr) return false;

    request.receivedOrder_ = nextReceivedOrder_++;
    request.status_ = RequestStatus::Queued;
    const size_t requestPosition = requests_.size();
    if (!drone_request_index_insert(&requestIndex_, id, requestPosition)) return false;
    requests_.push_back(std::move(request));

    // Put older waiting requests back in the heap so the C priority queue makes
    // the next dispatch decision across both old and new requests.
    retryWaitingRequests();

    DronePriorityItem item;
    item.request_id = id;
    item.urgency = static_cast<int>(requests_.back().severity());
    item.received_order = requests_.back().receivedOrder_;
    if (!drone_priority_queue_push(&priorityQueue_, item)) {
        requests_.back().status_ = RequestStatus::Waiting;
        if (!drone_request_queue_push(&pendingQueue_, id)) {
            record("Queue capacity reached; request " + std::to_string(id) + " could not be scheduled.");
            return false;
        }
        return true;
    }

    record("Request " + std::to_string(id) + " received: " + toString(requests_.back().severity()) +
           " " + toString(requests_.back().type()) + " at " + findZone(requests_.back().zoneId())->name() + ".");
    processPriorityQueue();
    return true;
}

bool DispatchManager::resolveRequest(int requestId) {
    Request* request = findRequest(requestId);
    if (request == nullptr || request->status_ != RequestStatus::Dispatched) return false;
    VolunteerTeam* team = findTeam(request->assignedTeamId_);
    if (team == nullptr) return false;

    request->status_ = RequestStatus::Resolved;
    team->release();
    record("Request " + std::to_string(requestId) + " resolved; " + team->id() + " is available at " +
           findZone(team->zoneId())->name() + ".");
    retryWaitingRequests();
    processPriorityQueue();
    return true;
}

Request* DispatchManager::findRequest(int requestId) {
    size_t position = 0;
    if (!drone_request_index_find(&requestIndex_, requestId, &position) || position >= requests_.size()) return nullptr;
    return &requests_[position];
}

const Request* DispatchManager::findRequest(int requestId) const {
    size_t position = 0;
    if (!drone_request_index_find(&requestIndex_, requestId, &position) || position >= requests_.size()) return nullptr;
    return &requests_[position];
}

VolunteerTeam* DispatchManager::findTeam(const std::string& teamId) {
    const auto found = std::find_if(teams_.begin(), teams_.end(), [&](const auto& team) {
        return team->id() == teamId;
    });
    return found == teams_.end() ? nullptr : found->get();
}

const Zone* DispatchManager::findZone(int zoneId) const {
    if (zoneId < 0 || static_cast<size_t>(zoneId) >= zones_.size()) return nullptr;
    return &zones_[static_cast<size_t>(zoneId)];
}

VolunteerTeam* DispatchManager::nearestAvailableTeam(const Request& request, double& distanceKm,
                                                       std::vector<int>& route) {
    VolunteerTeam* nearest = nullptr;
    distanceKm = 0.0;
    double shortest = 0.0;
    for (const auto& candidate : teams_) {
        if (!candidate->available() || candidate->type() != request.type()) continue;
        int path[DRONE_MAX_VERTICES];
        double candidateDistance = 0.0;
        const int pathLength = drone_graph_shortest_path(&graph_, candidate->zoneId(), request.zoneId(),
                                                         path, DRONE_MAX_VERTICES, &candidateDistance);
        if (pathLength <= 0 || (nearest != nullptr && candidateDistance >= shortest)) continue;
        nearest = candidate.get();
        shortest = candidateDistance;
        route.assign(path, path + pathLength);
    }
    if (nearest != nullptr) distanceKm = shortest;
    return nearest;
}

void DispatchManager::retryWaitingRequests() {
    int requestId = 0;
    while (drone_request_queue_pop(&pendingQueue_, &requestId)) {
        Request* request = findRequest(requestId);
        if (request == nullptr || request->status_ != RequestStatus::Waiting) continue;
        DronePriorityItem item;
        item.request_id = requestId;
        item.urgency = static_cast<int>(request->severity());
        item.received_order = request->receivedOrder_;
        if (!drone_priority_queue_push(&priorityQueue_, item)) {
            (void)drone_request_queue_push(&pendingQueue_, requestId);
            break;
        }
        request->status_ = RequestStatus::Queued;
    }
}

void DispatchManager::processPriorityQueue() {
    DronePriorityItem item;
    while (drone_priority_queue_pop(&priorityQueue_, &item)) {
        Request* request = findRequest(item.request_id);
        if (request == nullptr || request->status_ == RequestStatus::Resolved ||
            request->status_ == RequestStatus::Dispatched) continue;

        double distanceKm = 0.0;
        std::vector<int> route;
        VolunteerTeam* team = nearestAvailableTeam(*request, distanceKm, route);
        if (team == nullptr) {
            request->status_ = RequestStatus::Waiting;
            if (!drone_request_queue_push(&pendingQueue_, request->id())) {
                record("Pending queue capacity reached for request " + std::to_string(request->id()) + ".");
            }
            continue;
        }

        team->assign(request->id(), request->zoneId());
        request->status_ = RequestStatus::Dispatched;
        request->assignedTeamId_ = team->id();
        std::ostringstream message;
        message << "Request " << request->id() << " dispatched to " << team->id() << " ("
                << team->responseDescription() << ") via " << routeDescription(route) << " ["
                << formatDistance(distanceKm) << "].";
        record(message.str());
    }
}

void DispatchManager::record(const std::string& message) {
    (void)drone_history_append(&historyHead_, &historyTail_, message.c_str());
}

std::string DispatchManager::routeDescription(const std::vector<int>& route) const {
    std::ostringstream out;
    for (size_t i = 0; i < route.size(); ++i) {
        const Zone* zone = findZone(route[i]);
        if (i != 0) out << " -> ";
        out << (zone == nullptr ? "Unknown" : zone->name());
    }
    return out.str();
}

void DispatchManager::loadDemonstrationScenario() {
    addZone(Zone(0, "North Ridge"));
    addZone(Zone(1, "Old Town"));
    addZone(Zone(2, "Riverbend"));
    addZone(Zone(3, "Cedar Crossing"));
    addZone(Zone(4, "Lakeview"));
    addZone(Zone(5, "Southbank"));
    addZone(Zone(6, "Central Depot"));

    connectZones(0, 1, 4.0);
    connectZones(0, 3, 6.0);
    connectZones(1, 2, 3.0);
    connectZones(1, 4, 5.0);
    connectZones(2, 3, 2.0);
    connectZones(2, 5, 6.0);
    connectZones(3, 4, 3.0);
    connectZones(3, 6, 7.0);
    connectZones(4, 5, 4.0);
    connectZones(5, 6, 3.0);

    addTeam(std::make_unique<MedicalTeam>("M-01", "North Clinic", 1));
    addTeam(std::make_unique<RescueTeam>("R-01", "Rapid Rescue", 6));
    addTeam(std::make_unique<RescueTeam>("R-02", "River Rescue", 2));
    addTeam(std::make_unique<SupplyTeam>("S-01", "Relief Truck", 5));
    addTeam(std::make_unique<SupplyTeam>("S-02", "Depot Supplies", 6));

    submitRequest(Request(1048, RequestType::Medical, Severity::Critical, 0,
                          "Multiple injuries reported near the ridge", 0));
    submitRequest(Request(1047, RequestType::Medical, Severity::High, 4,
                          "First-aid support requested", 0));
    submitRequest(Request(1046, RequestType::Rescue, Severity::Critical, 3,
                          "Residents stranded after bridge flooding", 0));
    submitRequest(Request(1045, RequestType::Rescue, Severity::High, 0,
                          "Check homes after slope movement", 0));
    submitRequest(Request(1044, RequestType::Supply, Severity::High, 2,
                          "Drinking water needed at the school shelter", 0));
    submitRequest(Request(1043, RequestType::Rescue, Severity::Critical, 5,
                          "Evacuation support requested", 0));
    submitRequest(Request(1042, RequestType::Rescue, Severity::High, 4,
                          "Search team requested near the lake", 0));
}

void DispatchManager::printDashboard() const {
    size_t dispatched = 0, waiting = 0, resolved = 0;
    for (const Request& request : requests_) {
        if (request.status() == RequestStatus::Dispatched) ++dispatched;
        else if (request.status() == RequestStatus::Waiting) ++waiting;
        else if (request.status() == RequestStatus::Resolved) ++resolved;
    }

    std::cout << "\n=== D.R.O.N.E. Dispatch Dashboard ===\n"
              << "Zones: " << zones_.size() << " | Teams: " << teams_.size()
              << " | Requests: " << requests_.size() << " (" << dispatched << " dispatched, "
              << waiting << " waiting, " << resolved << " resolved)\n\n";

    std::cout << "REQUESTS\n";
    for (const Request& request : requests_) {
        const Zone* zone = findZone(request.zoneId());
        std::cout << "  #" << request.id() << " " << std::setw(8) << std::left << toString(request.severity())
                  << std::setw(9) << toString(request.type()) << std::setw(17) << toString(request.status())
                  << (zone == nullptr ? "Unknown" : zone->name());
        if (!request.assignedTeamId().empty()) std::cout << " | team " << request.assignedTeamId();
        std::cout << "\n";
    }

    std::cout << "\nTEAMS\n";
    for (const auto& team : teams_) {
        const Zone* zone = findZone(team->zoneId());
        std::cout << "  " << std::setw(5) << std::left << team->id() << " "
                  << std::setw(19) << team->name() << std::setw(9) << toString(team->type())
                  << (team->available() ? "Available" : "Assigned")
                  << " | " << (zone == nullptr ? "Unknown" : zone->name());
        if (!team->available()) std::cout << " | #" << team->assignedRequestId();
        std::cout << "\n";
    }
    std::cout << std::right;
}

void DispatchManager::printHistory() const {
    std::cout << "\nDISPATCH HISTORY\n";
    for (const DroneHistoryNode* node = historyHead_; node != nullptr; node = node->next) {
        std::cout << "  - " << node->message << "\n";
    }
}

const char* toString(RequestType type) {
    switch (type) {
        case RequestType::Medical: return "Medical";
        case RequestType::Rescue: return "Rescue";
        case RequestType::Supply: return "Supply";
    }
    return "Unknown";
}

const char* toString(Severity severity) {
    switch (severity) {
        case Severity::Low: return "Low";
        case Severity::Medium: return "Medium";
        case Severity::High: return "High";
        case Severity::Critical: return "Critical";
    }
    return "Unknown";
}

const char* toString(RequestStatus status) {
    switch (status) {
        case RequestStatus::Queued: return "Queued";
        case RequestStatus::Waiting: return "Waiting";
        case RequestStatus::Dispatched: return "Dispatched";
        case RequestStatus::Resolved: return "Resolved";
    }
    return "Unknown";
}

} // namespace drone
