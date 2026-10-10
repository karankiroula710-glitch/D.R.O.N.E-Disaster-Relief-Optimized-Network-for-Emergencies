#ifndef DRONE_MODELS_HPP
#define DRONE_MODELS_HPP

#include <string>

enum class RequestType { Medical = 1, Rescue = 2, Supply = 3 };
enum class Severity { Low = 1, Medium = 2, High = 3, Critical = 4 };
enum class RequestStatus { InInbox, Waiting, Assigned, Resolved };

struct Zone {
    int id;
    std::string name;
};

class Request {
public:
    Request(int id, RequestType type, Severity severity, int zoneId,
            std::string source, std::string description, unsigned long order);

    int id() const;
    RequestType type() const;
    Severity severity() const;
    int zoneId() const;
    const std::string& source() const;
    const std::string& description() const;
    unsigned long arrivalOrder() const;
    RequestStatus status() const;
    int assignedTeamId() const;

    void setStatus(RequestStatus status);
    void assignTo(int teamId);
    void clearAssignment();

private:
    int id_;
    RequestType type_;
    Severity severity_;
    int zoneId_;
    std::string source_;
    std::string description_;
    unsigned long arrivalOrder_;
    RequestStatus status_;
    int assignedTeamId_;
};

class VolunteerTeam {
public:
    VolunteerTeam(int id, std::string name, int zoneId);
    virtual ~VolunteerTeam() = default;

    int id() const;
    const std::string& name() const;
    int zoneId() const;
    bool isAvailable() const;
    void setAvailable(bool available);
    virtual RequestType specialty() const = 0;
    virtual std::string responseLabel() const = 0;

private:
    int id_;
    std::string name_;
    int zoneId_;
    bool available_;
};

class MedicalTeam final : public VolunteerTeam {
public:
    using VolunteerTeam::VolunteerTeam;
    RequestType specialty() const override;
    std::string responseLabel() const override;
};

class RescueTeam final : public VolunteerTeam {
public:
    using VolunteerTeam::VolunteerTeam;
    RequestType specialty() const override;
    std::string responseLabel() const override;
};

class SupplyTeam final : public VolunteerTeam {
public:
    using VolunteerTeam::VolunteerTeam;
    RequestType specialty() const override;
    std::string responseLabel() const override;
};

std::string toString(RequestType type);
std::string toString(Severity severity);
std::string toString(RequestStatus status);

#endif
