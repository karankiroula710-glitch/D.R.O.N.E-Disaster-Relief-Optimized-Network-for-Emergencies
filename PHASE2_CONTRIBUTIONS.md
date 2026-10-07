# Phase 2 contribution split

**Target:** Phase 2 represents 40% of the complete PBL project. This plan divides that target into four 10-percentage-point work packages.

> These percentages are a proposed allocation, not a line-count measurement, certification by the mentor, or proof that the named person has already completed the work. The shared starter code and the team-labeled baseline commit do not establish four individual contributions. Each person should make a meaningful change in their assigned area, understand and explain it, and commit it to GitHub while signed into their own account. Match the final record to the rubric and work actually completed.

## Owners and code areas

| Member | Planned share | Work package | Code locations | A concrete individual contribution to make |
| --- | ---: | --- | --- | --- |
| **Lakshita Gusain** (team lead) | 10% | Request modeling and intake | `core/cpp/include/dispatch_manager.hpp`: `Request`; `core/cpp/src/dispatch_manager.cpp`: `Request` methods, `submitRequest`, `findRequest`; `index.html` request dialog | Add validation (for example, reject blank descriptions or invalid IDs), then demonstrate a valid request being indexed and submitted. |
| **Prerna Shukla** | 10% | Scheduling and request lifecycle | `core/c/drone_structures.c`: priority heap and circular pending queue; `core/cpp/src/dispatch_manager.cpp`: `retryWaitingRequests`, `processPriorityQueue`, `resolveRequest` | Improve or document tie-breaking and retry behavior; demonstrate a freed matching team taking the highest-priority waiting request. |
| **Karan Singh Kiroula** | 10% | Zone network and shortest-route selection | `core/c/drone_structures.c`: graph and Dijkstra; `core/cpp/src/dispatch_manager.cpp`: `Zone`, `connectZones`, `nearestAvailableTeam`, `routeDescription` | Add a zone/link or route detail, then explain how the shortest path and nearest suitable team are chosen. |
| **Manishka Bisht** | 10% | Team OOP hierarchy and presentation views | `core/cpp/include/dispatch_manager.hpp`: `VolunteerTeam` subclasses; `core/cpp/src/dispatch_manager.cpp`: team implementations and dashboard; `core/cpp/src/main.cpp`; `app.js` dashboard/query | Extend one team's response description or improve a dashboard/query view, then demonstrate the resulting behavior. |

The C source file currently contains several data structures in one module, so Prerna and Karan should coordinate before editing that shared file. They can make separate commits for distinct changes, or split the implementation into smaller `.c` modules as part of their contribution.

## Individual GitHub records

The repository currently has a shared baseline commit credited to the project team. That is a team record; it does not create a contribution entry for each member. To show individual work:

1. Confirm each member has accepted access to the repository using their own GitHub account.
2. Each member signs into GitHub with their account and clones the repository or uses GitHub Desktop.
3. Each person edits their assigned area, runs the build, and commits their own changes with their account configured as the Git author.
4. Push each member's branch and open a pull request, or commit directly to the presentation branch if the team agrees.
5. Keep the pull request or commit links and a short explanation of the code each person changed for the mentor.

Do not share passwords or GitHub tokens. A team member should make and explain their own contribution; changing only commit author metadata does not make someone the author of work they did not do.

## Suggested commit and presentation plan

1. Use focused branches such as `phase2/lakshita-intake`, `phase2/prerna-dispatch`, `phase2/karan-routing`, and `phase2/manishka-teams-ui`.
2. Make one or more focused commits per person, for example `Validate request intake` or `Document priority queue tie-breaking`.
3. Review the four changes together and merge them into the presentation branch.
4. In the presentation, show each person's actual code change, one design decision, and a short demonstration.

## Presentation prompts

- **Lakshita:** “I worked on request modeling and intake. I can show the validation and how a request is indexed and submitted.”
- **Prerna:** “I worked on urgency ordering and team availability. I can show how the highest-priority waiting request is assigned when a team is released.”
- **Karan:** “I worked on the zone graph and route selection. I can trace the weighted shortest path and explain how the nearest suitable team is chosen.”
- **Manishka:** “I worked on the volunteer team classes and presentation views. I can explain inheritance and demonstrate a team-specific response.”

Use these prompts only after each member has personally completed or reviewed the corresponding contribution and can explain it in their own words.
