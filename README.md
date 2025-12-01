# UMBC Interactive Accessibility Map

The UMBC Interactive Accessibility Map program is an application designed for Windows and Linux devices that renders the 2025 UMBC Map onto a canvas and displays accessible routes for students and faculty to take.

For Administrators, the application provides built-in tools via a CLI that allows for the creation, modification, or deletion of any in-built map parameters and features. Refer to the CLI Program in the attached files.

For users, the UI application includes the ability to pathfind across campus by inputting start and end locations.
Begin by either selecting a starting position on the map, or typing in a more specific location in the search bar above.
Next, in the same fashion, input a destination location.
After clicking go, the program will instantly compute the most optimal route to take that has the most efficient cost to traverse from the starting location to the other.

Other features of the UI Application include:
- An output window that displays copyable text to describe how to travel from one location to the other.
- Accessibility filters that enable users to toggle whether or not they want to use exterior accessible routes like stairs and non-accessible exterior doors.
- A Google Form to provide UMBC-Authenticated feedback information on the application.
- References to the Facilities Management for repairs to accessible features that are considered in the application.
- A brief text tutorial explaining how to use the app.

The entire program is written in C/C++, with very limited external libraries or other imports. It utilizes the GTK4 library to render the application and support its entire frontend framework.
All map information that is stored is serialized in binary, meaning that all calculations and storage space is incredibly minimal on the user's machine.
Therefore, this application will work incredibly quickly and reliably without any need for an Internet connection.

We hope you enjoy the UMBC Interactive Accessibility Map!

To run as a developer or administrator, clone the repository. Then:

To run the CLI:
- `make run_cli`

To run the UI:
- Ensure you have gtk4 installed.
- `make run_ui`
