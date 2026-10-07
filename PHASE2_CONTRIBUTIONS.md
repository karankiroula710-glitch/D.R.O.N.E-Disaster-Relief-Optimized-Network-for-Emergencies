# Phase 2 contribution split

**Target:** Phase 2 represents 40% of the complete PBL project. The four work packages below divide that target into four 10-percentage-point slices.

> The percentages are a fair work-allocation plan, not a line-count measurement or proof of authorship. The current prototype is shared starter code. Each person should personally review and extend their assigned area, understand it well enough to explain it, and make their own GitHub commit before claiming that work as an individual contribution. Adjust these shares to match the course rubric and the work actually completed.

## Owners and code areas

| Member | Planned share | Own this part | Code locations | Individual deliverable |
| --- | ---: | --- | --- | --- |
| **Lakshita Gusain** (team lead) | 10% | Request intake and request lifecycle | `index.html` request dialog; `app.js`: `openRequestDialog`, `closeRequestDialog`, `submitRequest`, `saveState`, `resetDemo` | Improve the intake flow or validation; show a new report being stored and its assigned/queued result. |
| **Prerna Shukla** | 10% | Priority scheduling and team lifecycle | `app.js`: `PriorityQueue`, `processQueue`, `getQueue`, `resolveRequest`, `changeTeamStatus` | Extend or document urgency and tie-breaking behavior; demonstrate a released team picking up the highest-priority matching request. |
| **Karan Singh Kiroula** | 10% | Zone graph and shortest-route calculation | `app.js`: `ZONES`, `EDGES`, `shortestPaths`, `pathTo`, `nearestTeam`, `renderMap`, `renderRoutePanel`, `renderZoneTable` | Extend a zone or route detail; explain how the shortest path and nearest suitable team are selected. |
| **Manishka Bisht** | 10% | Dashboard, request/team views, and local query assistant | `app.js`: `renderMetrics`, `renderRequests`, `renderTeams`, `renderActivity`, `assistantAnswer`, `askQuestion`, `submitGlobalSearch` | Add a useful dashboard/query improvement; demonstrate filtering a report or answering a question from the current scenario. |

## Suggested GitHub workflow

1. Give each member their own branch: `phase2/lakshita-intake`, `phase2/prerna-dispatch`, `phase2/karan-routing`, or `phase2/manishka-dashboard`.
2. Each person makes a focused change in their assigned area and commits it from their own GitHub account. For example: `Improve request form validation`.
3. Review the changes together and merge the four contributions into the presentation branch.
4. In the presentation, describe the code each person actually changed, explain one design decision, and run that person's demonstration flow.

The code is currently integrated in `app.js` so the browser prototype can run as a single local page. The named function groups above are the handoff boundaries for Phase 2; they are not Git authorship labels. If the team later splits the JavaScript into separate modules, preserve these ownership boundaries and update this file.

## Presentation prompts

- **Lakshita:** “I own the report intake flow. I can show how a submitted request is checked, stored in this browser, and sent to the dispatch flow.”
- **Prerna:** “I own urgency ordering and team availability. I can show why a Critical request goes ahead of a High request and how a freed matching team takes the next one.”
- **Karan:** “I own the zone network and route selection. I can trace a shortest route through the weighted links and explain why the nearest suitable team is chosen.”
- **Manishka:** “I own the command views and local query assistant. I can filter the current requests and show how a question is answered from the scenario data.”

Use these as prompts only after each member has made or reviewed the corresponding contribution and can explain it in their own words.
