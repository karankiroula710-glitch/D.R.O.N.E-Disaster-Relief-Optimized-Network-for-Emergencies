# D.R.O.N.E.
### Disaster Relief Optimized Network for Emergencies

**PBL project · Interactive browser prototype and native C/C++ demonstration**  
**Team:** Resqnet · **Team number:** T160

D.R.O.N.E. is a classroom prototype for coordinating disaster-relief requests. It demonstrates how incoming reports can be classified by urgency, matched with a suitable medical, rescue, or supply team, and routed across a network of affected zones. Requests remain in a waiting queue when no suitable team is available.

The project has two demonstrations based on the same scenario:

- The **interactive frontend** is HTML, CSS, and JavaScript. Open `index.html` in a browser to demonstrate request intake, queries, and the command dashboard.
- The **native dispatch core** is C and C++. It demonstrates real C data structures called from a C++ object-oriented dispatch manager. Build and run its command-line scenario as described below.

The browser application is a standalone simulator. It does not call the native C/C++ program or store shared data on a server; show the browser and CLI as separate parts of the prototype.

## Project team

- Lakshita Gusain — Team lead
- Prerna Shukla
- Karan Singh Kiroula
- Manishka Bisht

## Project objectives

1. Capture emergency requests by type, affected zone, and urgency.
2. Process Critical requests before lower-priority requests, with earlier requests first at equal urgency.
3. Match Medical, Rescue, and Supply requests to teams with the same specialty.
4. Select the nearest suitable team through a shortest-path calculation over the zone network.
5. Keep requests waiting when no suitable team is free and retry them when a team returns to service.
6. Show requests, teams, zones, routes, and dispatch activity in an understandable interface and CLI demonstration.
7. Demonstrate C data structures, C++ classes and inheritance, and a C-to-C++ interface.

## Run the interactive frontend

1. Open `index.html` in a current desktop or mobile browser. No installation, build step, or internet connection is required.
2. Use **New request** to submit a Medical, Rescue, or Supply report.
3. Review the dispatch result on the Command center or Requests page. A suitable available team is assigned automatically; otherwise the report enters the priority queue.
4. Resolve a dispatched report from the Requests page to release its team. If that team type has a waiting request, the next request is dispatched automatically.
5. Select a zone on the response map to preview a route and use **Ask dispatch** for questions about the current simulation.
6. Choose **Reset demo data** in the sidebar to restore the starting scenario.

The browser stores demo changes in local storage. Reset the demo to return to the initial set of requests and teams. Submitted reports are classroom data; do not enter real personal or emergency information.

## Build and run the C/C++ demonstration

The build script expects GCC and G++ (MinGW-w64 on Windows) to be installed and available on `PATH`. It also checks the common `C:\MinGW\bin` installation folder.

Open PowerShell in the project folder and run:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\build.ps1
.\build\drone-demo.exe
```

The execution-policy change applies only to that PowerShell window. The CLI prints the starting requests and teams, then resolves a rescue request to show the highest-priority waiting rescue request being assigned to the newly available team.

## Native core design

### C data structures (`core/c/`)

- **Binary max-heap priority queue:** orders requests by urgency, then earlier receipt time.
- **Weighted adjacency-matrix graph:** stores zone links; Dijkstra's algorithm returns the shortest route and distance.
- **Circular pending queue:** holds requests that currently have no suitable available team.
- **Open-addressed hash index:** maps numeric request IDs to the C++ request collection.
- **Singly linked history list:** records request arrivals, dispatches, and resolutions.

### C++ object-oriented layer (`core/cpp/`)

- `Zone` and `Request` classes encapsulate scenario and incident data.
- Abstract `VolunteerTeam` defines common team state and a virtual response description.
- `MedicalTeam`, `RescueTeam`, and `SupplyTeam` specialize the base team using inheritance and method overriding.
- `DispatchManager` coordinates intake, priority ordering, specialty matching, shortest-path routing, queue retry, and resolution.
- `drone_structures.h` uses `extern "C"` guards so C++ can call the C functions with C linkage.

### Dispatch flow

```text
Request intake (C++)
       ↓
C priority heap: urgency, then received order
       ↓
Find an available team with the matching specialty
       ├── no suitable team → C circular pending queue
       └── team available → C Dijkstra route → C++ team assignment
                                              ↓
                                      resolve the request
                                              ↓
                                retry queued requests by priority
```

The CLI scenario uses predefined zones, links, teams, and requests. Distances are illustrative and do not use GPS, live roads, traffic, weather, or hazard feeds.

## Interactive frontend features

- Request intake for type, urgency, zone, reporter label, and description.
- Urgency queue with Critical, High, Medium, then Low priority.
- Specialty matching, shortest-route preview, and waiting queue.
- Team status controls and a dispatch activity log.
- Local query assistant for current scenario data. It uses preset local rules and does not call an AI service.
- Browser persistence through local storage and a responsive layout.

## Project structure

```text
drone-disaster-management/
├── index.html                  # Interactive frontend markup
├── styles.css                  # Responsive visual design
├── app.js                      # Browser scenario simulator
├── build.ps1                   # Build the native C/C++ CLI with GCC/G++
├── core/
│   ├── c/
│   │   ├── drone_structures.h  # C API shared with C++
│   │   └── drone_structures.c  # Heap, graph, queue, hash index, linked list
│   └── cpp/
│       ├── include/dispatch_manager.hpp
│       └── src/                # Dispatch manager and CLI demonstration
├── PHASE2_CONTRIBUTIONS.md     # Four-person Phase 2 work split
└── README.md
```

## Limits

- Zones, links, distances, teams, and starting incidents are predefined demonstration data.
- One team handles one open request at a time. A team can take another request after its current report is resolved.
- The browser data is kept only in the current browser profile. There is no server, shared team view, login, role permissions, or remote synchronization.
- The query assistant uses local keyword rules and current scenario data. It does not understand arbitrary natural language and is not an emergency operator.
- This is a project demonstration, not an operational disaster-management system. For a real emergency, contact the appropriate local emergency service.

## Suggested classroom demonstration

1. Show the browser intake form and submit a Critical request; explain that this frontend is a JavaScript presentation simulator.
2. Build and run the CLI. Point out the C heap, graph/Dijkstra routine, circular queue, hash index, and linked history.
3. Explain how C++ `DispatchManager` uses these structures and how team subclasses override `responseDescription()`.
4. Resolve request `#1046` in the CLI flow and show Critical waiting request `#1043` being dispatched before the lower-priority rescue request.
5. Describe only the contribution each member actually made and can explain. See [PHASE2_CONTRIBUTIONS.md](PHASE2_CONTRIBUTIONS.md) for the planned split and GitHub workflow.

## Source proposal

This project uses the title, team identity, problem framing, goals, and dispatch approach from the supplied D.R.O.N.E. proposal. The browser frontend is an added presentation layer; the C/C++ command-line program demonstrates the proposed data-structure and OOP design.
