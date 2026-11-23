# D.R.O.N.E.

## Disaster Relief Optimized Network for Emergencies

**PBL project · C and C++ · Team Resqnet · Team T160**

D.R.O.N.E. is a local dispatch simulation for a relief coordination point. The operator records reports that have reached the desk, prioritizes them, assigns available Medical, Rescue, or Supply teams, and gets a route suggestion through a sample zone graph.

## Native C/C++ program

The console application uses C data structures with a C++ dispatch workflow:

- A max-heap orders incoming reports by urgency, with arrival order breaking ties.
- A circular queue holds reports until a suitable team becomes available.
- A weighted graph and Dijkstra's algorithm suggest routes between sample places.
- A hash table supports report lookup; a linked list records events for the current run.
- C++ classes model reports and teams, including Medical, Rescue, and Supply team types.
- The sample roster has 10 fictional teams of each type. The Dehradun-themed graph starts with 28 places.
- During intake, the operator can add a place for the current session and connect it to an existing place with a demonstration distance.

### Build and run on Windows

Install GCC and G++ and ensure both commands work in PowerShell. From the repository folder, run:

```powershell
Set-ExecutionPolicy -Scope Process Bypass -Force
.\build.ps1
.\build\drone-phase2.exe
```

The script compiles the C modules as C11 and the application as C++17. Build output is ignored by Git.

## Browser presentation prototype

`index.html` is a separate visual demo. It shows report intake, priority processing, team assignment, queries, and sample places in a browser. It uses fictional data held in the current page session; it is not connected to the C/C++ program.

Open `index.html` in a browser, or start a local server from this folder:

```powershell
py -3 -m http.server 4173
```

Then visit `http://127.0.0.1:4173/`. No package installation is required. Refreshing the page resets its sample data.

## Limits

The places, teams, requests, and road weights are demonstration data. Routes are not verified directions. The program does not use GPS, satellite, radio, live network services, or a database. A report must first reach the operator, for example through a shelter desk, checkpoint, runner, or working radio relay. If no communication or physical relay is available, the program cannot know that someone needs help. This is a classroom simulation, not a replacement for official emergency services.

Use fictional information in demonstrations; do not enter real names, contact details, or live incident information.

## Team

- Lakshita Gusain — Team lead
- Prerna Shukla
- Karan Singh Kiroula
- Manishka Bisht

Repository history records changes by account. Each member should explain the work they personally completed; a shared file alone is not proof of individual authorship.
