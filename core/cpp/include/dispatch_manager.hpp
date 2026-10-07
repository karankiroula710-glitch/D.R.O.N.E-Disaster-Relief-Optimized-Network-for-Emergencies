#ifndef DRONE_DISPATCH_MANAGER_HPP
#define DRONE_DISPATCH_MANAGER_HPP

#include "drone_structures.h"

#include <memory>
#include <string>
#include <vector>

namespace drone {

enum class RequestType { Medical, Rescue, Supply };
enum class Severity { Low = 1, Medium = 2, High = 3, Critical = 4 };
enum class RequestStatus { Queued, Waiting, Dispatched, Resolved };

class Zone {
public:
    Zone(int id, std::string name);
    int id() const;
    const std::string& name() const;
private:
    int id_;
    std::string name_;
};

class Request {
public:
    Request(int id, RequestType type, Severity severity, int zoneId,
            std::string description, unsigned long long receivedOrder);
    int id() const;
    RequestType type() const;
    Severity severity() const;
    int zoneId() const;
    const std::string& description() const;
    RequestStatus status() const;
    const std::string& assignedTeamId() const;
private:
    friend class DispatchManager;
    int id_;
    RequestType type_;
    Severity severity_;
    int zoneId_;
    std::string description_;
    unsigned long long receivedOrder_;
    RequestStatus status_;
    std::string assignedTeamId_;
};

class VolunteerTeam {
public:
    VolunteerTeam(std::string id, std::string name, RequestType type, int zoneId);
    virtual ~VolunteerTeam() = default;
    const std::string& id() const;
    const std::string& name() const;
    RequestType type() const;
    int zoneId() const;
    bool available() const;
    int assignedRequestId() const;
    void assign(int requestId, int destinationZone);
    void release();
    virtual std::string responseDescription() const = 0;
private:
    std::string id_;
    std::string name_;
    RequestType type_;
    int zoneId_;
    bool available_;
    int assignedRequestId_;
};

class MedicalTeam final : public VolunteerTeam {
public:
    MedicalTeam(std::string id, std::string name, int zoneId);
    std::string responseDescription() const override;
};

class RescueTeam final : public VolunteerTeam {
public:
    RescueTeam(std::string id, std::string name, int zoneId);
    std::string responseDescription() const override;
};

class SupplyTeam final : public VolunteerTeam {
public:
    SupplyTeam(std::string id, std::string name, int zoneId);
    std::string responseDescription() const override;
};

class DispatchManager {
public:
    DispatchManager();
    ~DispatchManager();
    DispatchManager(const DispatchManager&) = delete;
    DispatchManager& operator=(const DispatchManager&) = delete;

    bool addZone(Zone zone);
    bool connectZones(int fromZone, int toZone, double distanceKm);
    void addTeam(std::unique_ptr<VolunteerTeam> team);
    bool submitRequest(Request request);
    bool resolveRequest(int requestId);
    void loadDemonstrationScenario();
    void printDashboard() const;
    void printHistory() const;

private:
    std::vector<Zone> zones_;
    std::vector<Request> requests_;
    std::vector<std::unique_ptr<VolunteerTeam>> teams_;
    DroneGraph graph_;
    DronePriorityQueue priorityQueue_;
    DroneRequestQueue pendingQueue_;
    DroneRequestIndex requestIndex_;
    DroneHistoryNode* historyHead_;
    DroneHistoryNode* historyTail_;
    unsigned long long nextReceivedOrder_;

    Request* findRequest(int requestId);
    const Request* findRequest(int requestId) const;
    VolunteerTeam* findTeam(const std::string& teamId);
    const Zone* findZone(int zoneId) const;
    VolunteerTeam* nearestAvailableTeam(const Request& request, double& distanceKm,
                                         std::vector<int>& route);
    void retryWaitingRequests();
    void processPriorityQueue();
    void record(const std::string& message);
    std::string routeDescription(const std::vector<int>& route) const;
};

const char* toString(RequestType type);
const char* toString(Severity severity);
const char* toString(RequestStatus status);

} // namespace drone

#endif
