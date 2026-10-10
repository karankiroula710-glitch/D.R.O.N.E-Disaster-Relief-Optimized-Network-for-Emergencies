#include "models.hpp"

#include <utility>

Request::Request(int id, RequestType type, Severity severity, int zoneId,
                 std::string source, std::string description,
                 unsigned long order)
    : id_(id),
      type_(type),
      severity_(severity),
      zoneId_(zoneId),
      source_(std::move(source)),
      description_(std::move(description)),
      arrivalOrder_(order),
      status_(RequestStatus::InInbox),
      assignedTeamId_(-1) {}

int Request::id() const {
    return id_;
}

RequestType Request::type() const {
    return type_;
}

Severity Request::severity() const {
    return severity_;
}

int Request::zoneId() const {
    return zoneId_;
}

const std::string& Request::source() const {
    return source_;
}

const std::string& Request::description() const {
    return description_;
}

unsigned long Request::arrivalOrder() const {
    return arrivalOrder_;
}

RequestStatus Request::status() const {
    return status_;
}

int Request::assignedTeamId() const {
    return assignedTeamId_;
}

void Request::setStatus(RequestStatus status) {
    status_ = status;
}

void Request::assignTo(int teamId) {
    assignedTeamId_ = teamId;
    status_ = RequestStatus::Assigned;
}

void Request::clearAssignment() {
    assignedTeamId_ = -1;
}

VolunteerTeam::VolunteerTeam(int id, std::string name, int zoneId)
    : id_(id),
      name_(std::move(name)),
      zoneId_(zoneId),
      available_(true) {}

int VolunteerTeam::id() const {
    return id_;
}

const std::string& VolunteerTeam::name() const {
    return name_;
}

int VolunteerTeam::zoneId() const {
    return zoneId_;
}

bool VolunteerTeam::isAvailable() const {
    return available_;
}

void VolunteerTeam::setAvailable(bool available) {
    available_ = available;
}

RequestType MedicalTeam::specialty() const {
    return RequestType::Medical;
}

std::string MedicalTeam::responseLabel() const {
    return "medical response";
}

RequestType RescueTeam::specialty() const {
    return RequestType::Rescue;
}

std::string RescueTeam::responseLabel() const {
    return "rescue response";
}

RequestType SupplyTeam::specialty() const {
    return RequestType::Supply;
}

std::string SupplyTeam::responseLabel() const {
    return "supply delivery";
}

std::string toString(RequestType type) {
    switch (type) {
        case RequestType::Medical:
            return "Medical";
        case RequestType::Rescue:
            return "Rescue";
        case RequestType::Supply:
            return "Supply";
    }
    return "Unknown";
}

std::string toString(Severity severity) {
    switch (severity) {
        case Severity::Low:
            return "Low";
        case Severity::Medium:
            return "Medium";
        case Severity::High:
            return "High";
        case Severity::Critical:
            return "Critical";
    }
    return "Unknown";
}

std::string toString(RequestStatus status) {
    switch (status) {
        case RequestStatus::InInbox:
            return "In intake inbox";
        case RequestStatus::Waiting:
            return "Waiting for a suitable team";
        case RequestStatus::Assigned:
            return "Assigned";
        case RequestStatus::Resolved:
            return "Resolved";
    }
    return "Unknown";
}
