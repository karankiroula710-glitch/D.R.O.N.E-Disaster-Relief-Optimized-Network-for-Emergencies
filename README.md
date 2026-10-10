# D.R.O.N.E.

## Disaster Relief Optimized Network for Emergencies

**PBL project · C and C++ · Team Resqnet · Team T160**

D.R.O.N.E. is being developed as a classroom dispatch simulation for a small relief coordination point. The planned console program will organize reports that have already reached an operator, order incoming requests by severity, match them to available Medical, Rescue, or Supply teams, suggest a route through a small predefined zone map, and keep unmatched requests visible.

## Phase 2 scope

The team's Phase 2 target is a middle-stage prototype: core C data structures integrated with a C++ object-oriented dispatch workflow and a command-line interface. The exact progress percentage depends on the course rubric; the team will confirm it with the mentor.

Planned C components:

- Priority heap for intake order, with severity first and arrival order to break ties.
- Circular queue for reports waiting for a suitable team.
- Weighted zone graph and Dijkstra route suggestions.
- Hash table for report-ID lookup and linked list for session event history.

Planned C++ components:

- `Request`, `Zone`, and `VolunteerTeam` models.
- Medical, Rescue, and Supply team subclasses demonstrating inheritance and polymorphism.
- A dispatch manager and interactive terminal menu for intake, assignment, resolution, waiting reports, and basic queries.

**The Phase 2 source files are not in the current branch yet.** The team is preparing separate C/C++ modules and will add them in its own commits. This branch currently contains the project README only.

## Limits and network-loss scenario

The application is intended to run locally without an internet connection after it has been installed. Reports still need to reach the operator: for example, a person may report at a shelter/checkpoint, or a trained runner or working radio operator may relay the information. The program has no radio or satellite integration and does not obtain GPS.

If every communication and physical relay path is unavailable, the system cannot know where people are or receive a remote request. Reported zones are not verified locations. The project is a supplementary classroom simulation, not a replacement for government emergency services or satellite systems, and it has not been field-tested.

## Planned later work

Possible Phase 3 improvements include saving and reloading sample data, persistent event history, dispatcher review and override, blocked-road scenarios, and evaluation in a controlled drill. Any live data or communication integration would need a clear safety, reliability, and privacy plan.

## Team

- Lakshita Gusain — Team lead
- Prerna Shukla
- Karan Singh Kiroula
- Manishka Bisht

Each member will document and explain the changes they personally make. A shared starting point or unchanged file is not proof of individual authorship. Use only fictional information in the demo; for real emergencies, contact the appropriate local emergency service.
