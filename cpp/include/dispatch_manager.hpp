#ifndef DRONE_DISPATCH_MANAGER_HPP
#define DRONE_DISPATCH_MANAGER_HPP

#include "drone_structures.h"
#include "models.hpp"

#include <memory>
#include <string>
#include <vector>

class DispatchManager {
public:
    DispatchManager();
    ~DispatchManager();
    DispatchManager(const DispatchManager&) = delete;
    DispatchManager& operator=(const DispatchManager&) = delete;

    int zoneCount() const;
    std::string zoneName(int zoneId) const;
    int submitReport(RequestType type, Severity severity, int zoneId,
                     const std::string& source, const std::string& description);
    void processInbox();
    void resolveRequest(int requestId);
    void printDashboard() const;
    void printZones() const;
    void printRequests() const;
    void printWaiting() const;
    void printHistory() const;
    void answerQuery(const std::string& query) const;

private:
    std::vector<std::unique_ptr<Request>> requests_;
    std::vector<std::unique_ptr<VolunteerTeam>> teams_;
    DronePriorityHeap inbox_;
    DronePendingQueue pending_;
    DroneZoneGraph graph_;
    DroneIdMap idIndex_;
    DroneEventHistory history_;
    int nextRequestId_;
    unsigned long nextArrivalOrder_;

    Request* findRequest(int requestId);
    const Request* findRequest(int requestId) const;
    VolunteerTeam* findTeam(int teamId);
    VolunteerTeam* nearestAvailableTeam(const Request& request, int* outDistance);
    void dispatchOrWait(Request& request);
    void retryWaiting();
    void printRequestDetails(const Request& request) const;
    void printAssignment(const Request& request, const VolunteerTeam& team, int distance) const;
    void recordEvent(const std::string& message);
};

#endif
