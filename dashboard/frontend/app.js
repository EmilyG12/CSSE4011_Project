function updateLeaderboard() {
    fetch("get_leaderboard.php")
        .then(response => {
            return response.json();
        })

        .then(data => {
            const tableBody = document.querySelector("#leaderboard tbody");
            tableBody.innerHTML = "";

            data.forEach((entry, index) => {

                const row = document.createElement("tr");
                row.classList.add(index % 2 === 0 ? "row-bg-yellow" : "row-bg-orange");

                const playerCell = document.createElement("td");
                playerCell.textContent = `${index + 1}. ${entry.player_name}`;

                const scoreCell = document.createElement("td");
                scoreCell.textContent = `${entry.score} pts`;

                row.appendChild(playerCell);
                row.appendChild(scoreCell);
                tableBody.appendChild(row);
            });
        })

        .catch(error => {
            console.error("Error:", error);
        });
}

function addScore(playerName, score) {
    fetch("update_leaderboard.php", {method: "POST",
            headers: {"Content-Type": "application/x-www-form-urlencoded"},
            body: `player_name=${encodeURIComponent(playerName)}&score=${score}`});
}

async function fetchLatestData() {

  try {

    // Collect information
    const response = await fetch('/api/get-latest-data');
    const data = await response.json();

    console.log('Received data:', data);

    // Receive fight data
    // Check who wins
    // Display game log
    // Increment winner score by one if in table
    // Set winner score to one if not in table
    // Update table if required

    if (data && typeof data.player_name === "string") {
          console.log("Updating Table");
          addScore(data.player_name, data.score);
          updateLeaderboard();
    }

  } catch (err) {
    console.error('Error fetching latest data:', err);
  }
}

fetchLatestData();
setInterval(fetchLatestData, 3000);
