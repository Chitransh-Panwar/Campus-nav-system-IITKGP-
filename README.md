# 🏫 Campus Navigation System 📍

A web-based navigation tool for a university campus(IIT KGP) that uses **BFS (Breadth-First Search)**, **DFS (Depth-First Search)**, and **Dijkstra’s Algorithm** to compute paths between campus locations. The project features an interactive map (powered by **Leaflet.js**) and a data processing script (in **Python**) that converts GeoJSON data into a graph of nodes and edges.

## Table of Contents
- [Features](#-features)
- [Project Structure](#project-structure)
- [Technologies Used](#%EF%B8%8F-technologies-used)
- [Setup and Installation](#setup-and-installation)
- [Usage](#usage)
  - [Live Demo](#live-demo)
- [Future Enhancements](#future-enhancements)
- [Acknowledgments](#acknowledgments)

---

## ✨ Features

1. **Interactive Map Interface**  
   - Users can view a campus map with selectable buildings and pathways.

2. **Multiple Pathfinding Algorithms**  
   - **BFS:** Finds the path by exploring all nodes at each depth before moving deeper (shortest path in terms of number of edges on an unweighted graph).  
   - **DFS:** Explores as far as possible along each branch (not guaranteed shortest route).  
   - **Dijkstra’s Algorithm:** Computes the shortest path based on distances (weighted edges), suitable for real distances on campus.

3. **Data Processing with Python**  
   - Converts campus GeoJSON(`data.geojson`) data into a graph representation (`iitkgp.json`) including node coordinates, edges, and distances.

4. **Validation Checks**  
   - Ensures users select valid start/end locations and an algorithm before computing routes.  
   - Prevents identical start/end choices.

5. **Modular Codebase**  
   - **`algo.cpp`** for pathfinding logic.
   - **`graph.js`** for managing the graph data structure collected by emscripten from graph.cpp (nodes and adjacency lists).
   - **`graph.cpp`** for managing the graph data structure (nodes and adjacency lists).
   - **`app.js`** for main application flow, map initialization, and UI event handling.  
   - **`myprocess.py`** for data processing.

---

## Project Structure

```
campus-navigation/
├── index.html              # Main HTML file for the user interface
├── styles.css          # Custom CSS for the UI      
├── graph.js/graph.wasm     # Graph class / data structure handling
├── app.js              # Main application logic, map setup, event handlers
├── data.geojson        # Original campus GeoJSON data
├── iitkgp.json       # Generated file with processed nodes and edges
├── myprocess.py      # Python script to convert data.geojson -> campus_nodes_edges.json
├── graph.cpp         #  This file convert into graph.js after using web assembly and emscripten 
└── README.md               # This README file
```

---
## 🛠️ Technologies Used

- **Front-End:**
  - **HTML5**, **CSS3**, **JavaScript**
  - **Leaflet.js** for map visualization

- **Back-End / Data Processing:**
  - **Python 3** with a custom script (`myprocess.py`) for generating graph data from GeoJSON

- **Algorithms:**
  - **BFS** (Breadth-First Search)
  - **DFS** (Depth-First Search)
  - **Dijkstra’s Algorithm**

---
## Setup and Installation

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/YourUsername/campus-navigation.git
   cd campus-navigation
   ```

2. **Serve the Application Locally:**  
   - You can use a local server (e.g., Python’s built-in server) to serve `index.html`:
     ```bash
     python -m http.server 8000
     ```
   - Then open `http://localhost:8000/index.html` in your web browser.

---
## Usage

1. **Select Start and End Locations:**
   - Use the dropdown menus under **Start Location** and **End Location**.  
   - Ensure these are different (the app will show an error if they’re the same).

2. **Choose an Algorithm:**
   - **BFS**: Explores nodes in layers; good for unweighted graphs but ignores real distances.  
   - **DFS**: Explores a single path deeply; not guaranteed shortest.  
   - **Dijkstra’s**: Considers distances and finds the true shortest path on weighted graphs.

3. **Click **Find Route**:**
   - The app calculates the route using your chosen algorithm.  
   - A polyline is drawn on the map indicating the path.
  
### Live Demo
https://chitransh-panwar.github.io/Campus-nav-system-IITKGP-/
Here's a quick Video demonstrating the main workflow:

  

https://github.com/user-attachments/assets/e3d27e27-7ef8-44e1-a08e-4e6d57c72d1d


     
## Future Enhancements

- **Display Path Statistics:**
  - Show total distance traveled and estimated walking time on the UI.
- **Accessibility Features:**
  - Indicate wheelchair-accessible routes or building entrances.
- **Real-Time Updates:**
  - Handle temporary blockages or construction routes.
 
## Acknowledgments

- **Leaflet.js:** For the interactive map components.  
- **OpenStreetMap & GeoJSON:** For geospatial data formats.  
- **Instructors & Peers:** For guidance and feedback on BFS, DFS, and Dijkstra’s implementations.

---

**Thank you for checking out the Campus Navigation System!**  
If you have any questions or suggestions, feel free to open an issue or submit a pull request on GitHub.

#Disclaimer 

  there is a small issue in this as i made this project as semester project thiswas mandatory to use cpp thats why i used graph.cpp and convert it into graph.js/graph.wasm ,you can skip this part you can directly write graph.js on your own it will also work .
