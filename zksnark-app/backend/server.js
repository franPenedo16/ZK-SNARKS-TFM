const express = require('express');
const cors = require('cors');
const dbObject = require('./db');
const util = require('util');
const promisedb = util.promisify(dbObject.db.get.bind(dbObject.db));
const fs = require('fs');
const snarkjs = require('snarkjs');
const jwt = require('jsonwebtoken');
const jwtDecode = require('jwt-decode').jwtDecode;
const axios = require('axios');
require('dotenv').config();

const app = express();
const PORT = process.env.PORT || 3500;
const JWT_SECRET = process.env.JWT_SECRET || 'default_secret';

app.use(cors());
app.use(express.json());

function verificarToken(req, res, next) {
  const token = req.headers.authorization?.split(' ')[1];
  if (!token) return res.status(401).json({ error: 'Token Required' });

  jwt.verify(token, JWT_SECRET, (err, user) => {
    if (err) return res.status(403).json({ error: 'Token invalid' });
    req.user = user;
    next();
  });
}

function checkExistence(userId, objctId, table) {
  let query = '';
  if (table === 'resolved') query = 'SELECT * FROM resolved WHERE user = ? AND sudoku = ? LIMIT 1';

  if (table === 'verifications') query = 'SELECT * FROM verifications WHERE verifier = ? AND solution = ? LIMIT 1';

  if (table === 'blocks') query = 'SELECT * FROM blocks WHERE user = ? AND blocked_user = ? LIMIT 1';

  return new Promise((resolve, reject) => {
    dbObject.db.get(query, [userId, objctId], (err, row) => {
      if (err) {
        return reject(err);
      }
      resolve(!!row);
    });
  });
}

app.get('/api/users', verificarToken, (req, res) => {
  dbObject.db.all('SELECT id, hash FROM users', [], (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

app.get('/api/user/', (req, res) => {
  const dni = req.query.dni;
  const query = 'SELECT id, hash FROM users WHERE dni = ? LIMIT 1';
  dbObject.db.get(query, [dni], (err, row) => {
    if (err) {
      console.error('Error searching user:', err.message);
      res.status(500).json({ error: 'Error' });
    } else if (!row) {
      res.status(404).json({ error: 'User not found' });
    } else {
      res.json(row);
    }
  });
});


app.post('/api/login', async (req, res) => {
  if (!req.body || typeof req.body !== 'object') {
    return res.status(400).json({ error: 'Invalid json' });
  }

  const { public_data, proof_data, dni } = req.body;

  try {
    const user = await promisedb('SELECT * FROM users WHERE dni = ?', [dni]);
    if (!user) {
      return res.status(401).json({ error: 'User not exist' });
    }

    const publicSignals = JSON.parse(Buffer.from(public_data, 'base64').toString());
    const proof = JSON.parse(Buffer.from(proof_data, 'base64').toString());
    const nowSeconds = Math.floor(Date.now() / 1000);
    const diff = nowSeconds - publicSignals[2];

    if (diff > 300) {
      return res.status(401).json({ error: 'Expired proof' });
    }

    const vKey = JSON.parse(fs.readFileSync('circuits/auth/verification_key.json'));
    const isValid = await snarkjs.groth16.verify(vKey, publicSignals, proof);

    if (!isValid) {
      return res.status(401).json({ error: 'Invalid proof' });
    }

    const token = jwt.sign(
      { id: user.id, dni: user.dni, username: user.username },
      JWT_SECRET,
      { expiresIn: '0.5h' }
    );
    res.json({ token });

  } catch (err) {
    console.error('Error in /api/login:', err);
    res.status(500).json({ error: 'Error in login' });
  }
});


app.get('/api/sudoku/', verificarToken, async (req, res) => {

  const date = req.query.date;
  const level = req.query.level;
  const validLevels = ['easy', 'medium', 'hard'];

  if (!date || !level) {
    return res.status(400).json({ error: 'Invalid parameters' });
  }

  if (!validLevels.includes(level.toLowerCase())) {
    return res.status(400).json({ error: 'Invalid level. Must be easy, medium or hard' });
  }

  const query = 'SELECT * FROM sudokus WHERE date = ? AND level = ?';
  dbObject.db.get(query, [date, level], async (err, row) => {
    if (err) {
      console.error('Error searching sudoku:', err.message);
      return res.status(500).json({ error: 'Error' });
    }
    if (row) {
      return res.json({
        'date': row.date,
        'level': row.level,
        'problem': JSON.parse(row.problem)
      });
    } else {
      try {
        const apiKey = process.env.TOKEN_NINJA;
        let seed = 0;
        if (level === 'easy') {
          seed = date + 1;
        } else if (level === 'medium') {
          seed = date + 2;
        } else {
          seed = date + 3;
        }
        const response = await axios.get(`https://api.api-ninjas.com/v1/sudokugenerate?difficulty=${level}&seed=${seed}`, {
          headers: { 'X-Api-Key': apiKey },
        });

        const puzzleStr = JSON.stringify(response.data.puzzle).replaceAll('null', '0');
        const solutionStr = JSON.stringify(response.data.solution);

        const insertQuery = 'INSERT INTO sudokus (date, level, problem, solution) VALUES (?, ?, ?, ?)';
        dbObject.db.run(insertQuery, [date, level, puzzleStr, solutionStr]);

        res.json({
          'date': date,
          'level': level,
          'problem': JSON.parse(puzzleStr)
        });

      } catch (apiErr) {
        console.error('Error fetching from API Ninjas:', apiErr.message);
        res.status(404).json({ error: 'Sudoku not found externally' });
      }
    }
  });
});

app.post('/api/solve', verificarToken, async (req, res) => {
  if (!req.body || typeof req.body !== 'object') {
    return res.status(400).json({ error: 'Invalid json' });
  }

  const { public_data, proof_data, date, level } = req.body;

  try {
    const query = 'SELECT * FROM sudokus WHERE date = ? AND level = ?';
    dbObject.db.get(query, [date, level], async (err, row) => {
      if (err) {
        console.error('Error searching sudoku:', err.message);
        return res.status(500).json({ error: 'Error' });
      }

      if (!row) {
        return res.status(404).json({ error: 'Sudoku not found in db' });
      }

      const proof_data_str = Buffer.from(proof_data, 'base64').toString();
      const public_data_str = Buffer.from(public_data, 'base64').toString();

      try {
        const exists = await checkExistence(req.user.id, row.id, 'resolved');
        if (exists) {
          return res.status(409).json({ error: 'The proof already exists' });
        }

        const insertQuery = 'INSERT INTO resolved (user, sudoku, proof, public_inputs) VALUES (?, ?, ?, ?)';
        dbObject.db.run(insertQuery, [req.user.id, row.id, proof_data_str, public_data_str], (err) => {
          if (err) {
            console.error('Error saving proof:', err.message);
            return res.status(500).json({ error: 'Error saving proof' });
          }
          return res.json({ ok: 'Sudoku solution proof saved' });
        });

      } catch (checkErr) {
        console.error('Error during checkExistence:', checkErr.message);
        return res.status(500).json({ error: 'Error checking existence' });
      }
    });
  } catch (err) {
    console.error('Error in /api/resolve:', err);
    return res.status(500).json({ error: 'Error in resolve' });
  }
});

app.get('/api/resolved/', verificarToken, async (req, res) => {
  const query = `
    SELECT r.id AS id, u.username AS username, s.date AS date, s.level AS level,
           COUNT(DISTINCT u2.id) AS verifiers_count,
           '[' || COALESCE(GROUP_CONCAT(DISTINCT '"' || u2.username || '"'),'') || ']' AS verifiers_username
    FROM resolved AS r
    JOIN users AS u ON r.user = u.id
    JOIN sudokus AS s ON r.sudoku = s.id
    LEFT JOIN verifications AS v ON v.solution = r.id
    LEFT JOIN users AS u2 ON u2.id = v.verifier AND u2.id NOT IN ( SELECT blocked_user FROM blocks WHERE user = ? )
    GROUP BY r.id
    ORDER BY s.date
  `;
  dbObject.db.all(query, [req.user.id], (err, rows) => {

    if (err) return res.status(500).json({ error: err.message });
    rows.map((row) => {
      const dateStr = row.date.toString();
      row.date = `${dateStr.slice(0, 4)}-${dateStr.slice(4, 6)}-${dateStr.slice(6, 8)}`;
      row.verifiers_username = JSON.parse(row.verifiers_username);
    })
    res.json(rows);

  });
});

app.get('/api/solution/', verificarToken, async (req, res) => {
  const id = req.query.id;
  const query = `SELECT r.id, r.proof, r.public_inputs, u.id AS user_id, u.hash AS user_hash, s.problem AS problem FROM resolved AS r 
                  JOIN users AS u ON r.user = u.id JOIN sudokus AS s ON r.sudoku = s.id WHERE r.id = ? LIMIT 1`;
  dbObject.db.get(query, [id], (err, row) => {
    if (err) {
      console.error('Error searching user:', err.message);
      res.status(500).json({ error: 'Error' });
    } else if (!row) {
      res.status(404).json({ error: 'Solution not found' });
    } else {
      res.json({
        'proof': JSON.parse(row.proof),
        'public_inputs': JSON.parse(row.public_inputs),
        'user_id': row.user_id,
        'user_hash': row.user_hash,
        'problem': JSON.parse(row.problem),
        'resolved_id': row.id
      });
    }
  });
});

app.post('/api/verify', verificarToken, async (req, res) => {
  if (!req.body || typeof req.body !== 'object') {
    return res.status(400).json({ error: 'Invalid json' });
  }

  const { solved } = req.body;

  try {
    const query = 'SELECT * FROM resolved WHERE id = ?';
    dbObject.db.get(query, [solved], async (err, row) => {
      if (err) {
        console.error('Error searching solution:', err.message);
        return res.status(500).json({ error: 'Error' });
      }

      if (!row) {
        return res.status(404).json({ error: 'Solution not found in db' });
      }

      if (req.user.id === row.user) {
        return res.status(409).json({ error: "A user can't verify itself" });
      }

      try {
        const exists = await checkExistence(req.user.id, row.id, 'verifications');
        if (exists) {
          return res.status(409).json({ error: 'The validation already exists' });
        }

        const insertQuery = 'INSERT INTO verifications (verifier, solution) VALUES (?, ?)';
        dbObject.db.run(insertQuery, [req.user.id, solved], (err) => {
          if (err) {
            console.error('Error saving verification:', err.message);
            return res.status(500).json({ error: 'Error saving verification' });
          }
          return res.json({ ok: 'Sudoku verification saved' });
        });

      } catch (checkErr) {
        console.error('Error during checkExistence:', checkErr.message);
        return res.status(500).json({ error: 'Error checking existence' });
      }
    });
  } catch (err) {
    console.error('Error in /api/verify:', err);
    return res.status(500).json({ error: 'Error in verify' });
  }
});

app.get('/api/blocked/', verificarToken, (req, res) => {

  const query = 'SELECT b.id, u.username FROM blocks AS b JOIN users AS u ON b.blocked_user = u.id  WHERE b.user = ? ';
  dbObject.db.all(query, [req.user.id], (err, rows) => {
    if (err) {
      console.error('Error searching user:', err.message);
      res.status(500).json({ error: 'Error' });
    } else if (!rows) {
      res.json([])
    } else {
      res.json(rows);
    }
  });
});

app.post('/api/blocked', verificarToken, async (req, res) => {
  if (!req.body || typeof req.body !== 'object') {
    return res.status(400).json({ error: 'Invalid json' });
  }

  const { username } = req.body;

  try {
    const query = "SELECT * FROM users WHERE REPLACE(LOWER(username), ' ', '') = REPLACE(LOWER(?), ' ', '')";
    dbObject.db.get(query, [username], async (err, row) => {
      if (err) {
        console.error('Error searching user:', err.message);
        return res.status(500).json({ error: 'Error' });
      }

      if (!row) {
        return res.status(404).json({ error: 'User to block not found in db' });
      }

      if (req.user.id === row.id) {
        return res.status(409).json({ error: "A user can't block itself" });
      }

      try {
        const exists = await checkExistence(req.user.id, row.id, 'blocks');
        if (exists) {
          return res.status(409).json({ error: 'The user is already blocked' });
        }

        const insertQuery = 'INSERT INTO blocks (user, blocked_user) VALUES (?, ?)';
        dbObject.db.run(insertQuery, [req.user.id, row.id], (err) => {
          if (err) {
            console.error('Error saving blocked user:', err.message);
            return res.status(500).json({ error: 'Error saving verification' });
          }
          return res.json({ ok: 'Blocked user saved' });
        });

      } catch (checkErr) {
        console.error('Error during checkExistence:', checkErr.message);
        return res.status(500).json({ error: 'Error checking existence' });
      }
    });
  } catch (err) {
    console.error('Error in /api/blocked', err);
    return res.status(500).json({ error: 'Error in blocked' });
  }
});

app.delete('/api/blocked', verificarToken, async (req, res) => {
  if (!req.body || typeof req.body !== 'object') {
    return res.status(400).json({ error: 'Invalid json' });
  }

  const { id } = req.body;

  try {
    const query = "SELECT * FROM blocks WHERE id = ?";
    dbObject.db.get(query, [id], async (err, row) => {
      if (err) {
        console.error('Error searching blovk user:', err.message);
        return res.status(500).json({ error: 'Error' });
      }

      if (!row) {
        return res.status(404).json({ error: 'User is not block' });
      }


      if (req.user.id !== row.user) {
        return res.status(409).json({ error: "Can't unblock this user" });
      }

      try {
        const deleteQuery = 'DELETE FROM blocks WHERE id = ?';
        dbObject.db.run(deleteQuery, [row.id], (err) => {
          if (err) {
            console.error('Error deleting blocked user:', err.message);
            return res.status(500).json({ error: 'Error deleting blocked user' });
          }
          return res.json({ ok: 'User successfully unblocked' });
        });

      } catch (checkErr) {
        console.error('Error during checkExistence:', checkErr.message);
        return res.status(500).json({ error: 'Error checking existence' });
      }
    });
  } catch (err) {
    console.error('Error in DELETE /api/blocked', err);
    return res.status(500).json({ error: 'Error in unblocking' });
  }
});




app.listen(PORT, () => {
  dbObject.initDatabase();
  console.log(`Server running on http://localhost:${PORT}`);
});
