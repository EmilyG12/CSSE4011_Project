function updateLeaderboard() {
    fetch("test.php")
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

    if (data && "player_name" in data) {
          console.log("Updating Table");
          addScore(data.player_name, data.score);
          updateLeaderboard();
    } else if ("player1" in data) {
         console.log("Doing stuff");
         //document.getElementById('Player1').innerText = `${data.player1?.name ?? 'none'} Distance`;
         //document.getElementById('Player2').innerText = `${data.player2?.name ?? 'none'} Distance`;
         document.getElementById('DistanceM1').innerText = `${data.player1?.m ?? 'none'}m`;
         document.getElementById('DistanceM2').innerText = `${data.player2?.m ?? 'none'}m`;
         document.getElementById('DistanceC1').innerText = `${data.player1?.cms ?? 'none'}cm`;
         document.getElementById('DistanceC2').innerText = `${data.player2?.cms ?? 'none'}cm`;
    }

  } catch (err) {
    console.error('Error fetching latest data:', err);

  }
}


fetchLatestData();
setInterval(fetchLatestData, 3000);
