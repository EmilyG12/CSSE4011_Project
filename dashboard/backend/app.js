const express = require('express');
const path = require('path');
const app = express();
const port = 8081;

app.set('trust proxy', 'loopback');
app.use(express.json());

let latestData = { message: 'No data yet' };

app.post('/api/post-data', (req, res) => {
  latestData = req.body;
  console.log('Updated latestData:', latestData);
  res.json({ status: 'Success', received: req.body });
});
app.use(express.static(path.join(__dirname, '../htdocs')));

app.get('/api/get-latest-data', (req, res) => {
  res.json(latestData);
});

app.listen(port, () => {
  console.log(`Example app listening on port ${port}!`);
});
