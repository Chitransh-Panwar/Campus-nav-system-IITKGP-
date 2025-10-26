// Wait for the WebAssembly Module (graph.js / graph.wasm) to load
Module().then((Module) => {
  console.log("✅ WebAssembly module loaded successfully");

  // Fetch the campus graph data
  fetch("iitkgp.json")
    .then((response) => response.json())
    .then((data) => {
      console.log(`📦 Loaded ${data.nodes.length} nodes and ${data.edges.length} edges`);

      // Create a new C++ graph object
      const g = new Module.graph();

      // --------------------------------
      // Add nodes to the C++ graph
      // --------------------------------
      data.nodes.forEach((node) => {
        const lng = node.lng !== undefined ? node.lng : node.lon; // support both lon/lng
        if (node.id !== undefined && node.lat !== undefined && lng !== undefined) {
          g.addnode(node.id, node.lat, lng);
        } else {
          console.warn("⚠️ Skipping invalid node:", node);
        }
      });

      // --------------------------------
      // Add edges to the C++ graph
      // --------------------------------
      data.edges.forEach((edge) => {
        const weight = edge.weight !== undefined ? edge.weight : edge.distance;
        if (
          edge.from !== undefined &&
          edge.to !== undefined &&
          weight !== undefined &&
          edge.accessible !== undefined
        ) {
          g.addedge(edge.from, edge.to, weight, edge.accessible);
        } else {
          console.warn("⚠️ Skipping invalid edge:", edge);
        }
      });

      // --------------------------------
      // Group nodes by name
      // --------------------------------
      const nodesByName = {};
      data.nodes.forEach((node) => {
        const name =
          node.name && node.name.trim() !== "" ? node.name.trim() : `Node ${node.id}`;
        if (!nodesByName[name]) nodesByName[name] = [];
        nodesByName[name].push(node);
      });

      // --------------------------------
      // Compute average coordinates for each location
      // --------------------------------
      const locationMarkers = [];
      for (const name in nodesByName) {
        const nodes = nodesByName[name];
        const avgLat = nodes.reduce((sum, n) => sum + n.lat, 0) / nodes.length;
        const avgLng =
          nodes.reduce((sum, n) => sum + (n.lng ?? n.lon), 0) / nodes.length;
        locationMarkers.push({ name, lat: avgLat, lng: avgLng });
      }

      // --------------------------------
      // Prepare markers but don't show them yet
      // --------------------------------
      const markerMap = {};
      locationMarkers.forEach((location) => {
        const marker = L.marker([location.lat, location.lng]).bindPopup(location.name);
        markerMap[location.name] = marker;
      });

      // --------------------------------
      // 🟢 Add "Show/Hide All Locations" toggle
      // --------------------------------
      const allMarkers = Object.values(markerMap);
      let markersVisible = false;

      // Create a toggle button dynamically
      let toggleBtn = document.getElementById("toggleMarkers");
      if (!toggleBtn) {
        toggleBtn = document.createElement("button");
        toggleBtn.id = "toggleMarkers";
        toggleBtn.textContent = "show all locations";
        document.querySelector("body").appendChild(toggleBtn);
        toggleBtn.style.position = "absolute";
        toggleBtn.style.top = "10px";
        toggleBtn.style.right = "10px";
        toggleBtn.style.zIndex = "1000";
        toggleBtn.style.padding = "6px 12px";
        toggleBtn.style.background = "#fff";
        toggleBtn.style.border = "1px solid #ccc";
        toggleBtn.style.borderRadius = "8px";
        toggleBtn.style.cursor = "pointer";
        toggleBtn.style.boxShadow = "0 1px 4px rgba(0,0,0,0.3)";
      }

      toggleBtn.addEventListener("click", () => {
        if (!markersVisible) {
          allMarkers.forEach((m) => m.addTo(map));
          toggleBtn.textContent = "hide all locations";
          const bounds = L.latLngBounds(allMarkers.map((m) => m.getLatLng()));
          map.fitBounds(bounds);
        } else {
          allMarkers.forEach((m) => map.removeLayer(m));
          toggleBtn.textContent = "show all locations";
        }
        markersVisible = !markersVisible;
      });

      // --------------------------------
      // Dropdown setup
      // --------------------------------
      const startSelect = document.getElementById("start");
      const endSelect = document.getElementById("end");

      locationMarkers.forEach((location) => {
        const option = document.createElement("option");
        option.value = location.name;
        option.text = location.name;
        startSelect.add(option.cloneNode(true));
        endSelect.add(option);
      });

      // --------------------------------
      // Draw Path
      // --------------------------------
      let currentPathLayer = null;
      let activeMarkers = [];

      function drawPath(nodeIds, startName, endName) {
        if (currentPathLayer) map.removeLayer(currentPathLayer);
        activeMarkers.forEach((m) => map.removeLayer(m));
        activeMarkers = [];

        const latlngs = nodeIds.map((id) => {
          const node = data.nodes.find((n) => n.id === id);
          return [node.lat, node.lng ?? node.lon];
        });
        currentPathLayer = L.polyline(latlngs, { color: "red" }).addTo(map);

        const startLocation = locationMarkers.find((l) => l.name === startName);
        const endLocation = locationMarkers.find((l) => l.name === endName);

        if (startLocation) {
          const startMarker = L.marker([startLocation.lat, startLocation.lng], {
            icon: L.icon({
              iconUrl: "https://maps.google.com/mapfiles/ms/icons/green-dot.png",
              iconSize: [32, 32],
              iconAnchor: [16, 32],
            }),
          }).bindPopup(`Start: ${startLocation.name}`).addTo(map);
          activeMarkers.push(startMarker);
        }

        if (endLocation) {
          const endMarker = L.marker([endLocation.lat, endLocation.lng], {
            icon: L.icon({
              iconUrl: "https://maps.google.com/mapfiles/ms/icons/red-dot.png",
              iconSize: [32, 32],
              iconAnchor: [16, 32],
            }),
          }).bindPopup(`End: ${endLocation.name}`).addTo(map);
          activeMarkers.push(endMarker);
        }

        map.fitBounds(currentPathLayer.getBounds());
      }

      // --------------------------------
      // Clear path when location changes
      // --------------------------------
      [startSelect, endSelect].forEach((sel) => {
        sel.addEventListener("change", () => {
          if (currentPathLayer) {
            map.removeLayer(currentPathLayer);
            currentPathLayer = null;
          }
        });
      });

      // --------------------------------
      // Handle "Find Route"
      // --------------------------------
      document.getElementById("findRoute").addEventListener("click", () => {
        const startName = startSelect.value;
        const endName = endSelect.value;
        const algorithm = document.getElementById("algorithm").value;
        const accessibility = document.getElementById("accessibility").checked;

        if (startName === endName) {
          alert("⚠️ Start and end locations cannot be the same.");
          return;
        }

        const startIds = nodesByName[startName].map((n) => n.id);
        const endIds = nodesByName[endName].map((n) => n.id);

        let bestPath = null;
        let shortest = Infinity;

        for (const sid of startIds) {
          for (const eid of endIds) {
            let path = null;
            switch (algorithm) {
              case "bfs":
                path = Module.bfs(g, sid, eid);
                break;
              case "dfs":
                path = Module.dfs(g, sid, eid);
                break;
              case "dijkstra":
                path = Module.dijkstra(g, sid, eid, accessibility);
                break;
            }

            if (path && path.size() > 0) {
              const jsPath = [];
              for (let i = 0; i < path.size(); i++) jsPath.push(path.get(i));
              path.delete();

              if (jsPath.length < shortest) {
                shortest = jsPath.length;
                bestPath = jsPath;
              }
            }
          }
        }

        if (bestPath) {
          drawPath(bestPath, startName, endName);
        } else {
          alert("❌ No path found between the selected locations.");
        }
      });
    })
    .catch((err) => console.error("💥 Error loading data:", err));
});
