# D.R.O.N.E.

## Disaster Relief Optimized Network for Emergencies

**PBL project · C and C++ · Team Resqnet · Team T160**

D.R.O.N.E. is a classroom dispatch simulation for a local relief coordination point. Its native console code organizes reports that have reached an operator, prioritizes requests, matches them with available Medical, Rescue, or Supply teams, and suggests routes through sample zones.

## Current project files

- `priority_queue.c`, `pending_queue.c`, `zone_graph.c`, and `drone_structure.c` contain C data-structure modules.
- `cpp/` contains the C++ models, dispatch manager, and console entry point.
- `index.html` is a separate browser presentation prototype. It demonstrates report intake, dispatch, local queries, teams, and sample places with fictional in-memory data. It is not connected to the C/C++ console program.

## Run the browser presentation prototype

Open `index.html` in a browser. Alternatively, open PowerShell in this repository folder and run:

```powershell
py -3 -m http.server 4173
```

Then open `http://127.0.0.1:4173/`. The browser prototype requires no package installation. Refreshing the page resets its sample session.

## Limits

The sample reports, teams, locations, and route weights are for demonstration only. The prototype does not use GPS, satellite, radio, live network services, or a database. Reports still need to reach an operator, for example through a shelter desk, checkpoint, runner, or working radio relay. If no communication or physical relay is available, the program cannot know a person needs help. Route links and distances are illustrative, not verified navigation. This project is not a replacement for official emergency services.

Use fictional information in demonstrations. Do not enter real names, contact details, or live incident information.

## Team

- Lakshita Gusain — Team lead
- Prerna Shukla
- Karan Singh Kiroula
- Manishka Bisht

Each member should explain the work they personally completed and use the repository history as evidence of their own changes.
