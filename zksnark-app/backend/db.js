const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const dbPath = path.resolve(__dirname, 'database.db');
const db = new sqlite3.Database('./database.db');

const usuariosSeed = [
  {
    username: 'Elena Torres',
    dni: '29837456F',
    hash: '20068267145770480758598425271548912766311841084714144060121325998276043984585' // 123456
  },
  {
    username: 'Raúl Díaz',
    dni: '58293017C',
    hash: '6367127991333358977578960480704253828880607272705528183385701711452405926107' // poiuy
  },
  {
    username: 'Isabel Romero',
    dni: '36982017Y',
    hash: '11263359498448461489406253472212897113487008661809882393673843421825244582756' // asdfgh
  },
  {
    username: 'Pedro Navarro',
    dni: '47928103L',
    hash: '14593409748536177272731976671643771607807936675772333513630283222280012943705' // qwerty
  },
  {
    username: 'Fran Penedo',
    dni: '92196550S',
    hash: '12369028571331403061934813785835122137290300771186659783522312701150475204111' // pass: abcd
  }
]

function initDatabase() {
  return new Promise((resolve, reject) => {
    const db = new sqlite3.Database(dbPath, err => {
      if (err) return reject(err);
      db.serialize(() => {
        db.run(`
          CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            dni TEXT NOT NULL UNIQUE,
            hash TEXT NOT NULL
          )
        `, err => {
          if (err) return reject(err);

          db.get(`SELECT COUNT(*) AS cnt FROM users`, (err, row) => {
            if (err) return reject(err);

            if (row.cnt === 0) {
              const stmt = db.prepare(`INSERT INTO users (username, dni, hash) VALUES (?, ?, ?)`);
              for (const u of usuariosSeed) {
                stmt.run(u.username, u.dni, u.hash);
              }
              stmt.finalize(err => {
                if (err) return reject(err);
                resolve(db);
              });
            } else {
              resolve(db);
            }
          });
        });
      });
      db.run(`
        CREATE TABLE IF NOT EXISTS sudokus (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          problem TEXT NOT NULL,
          solution TEXT NOT NULL,
          date INTEGER NOT NULL,
          level TEXT NOT NULL CHECK(level IN ('easy','medium','hard'))
        )
      `, err => {
        if (err) return reject(err);
      });
      db.run(`
        CREATE TABLE IF NOT EXISTS resolved (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          user INTEGER,
          sudoku INTEGER,
          proof TEXT NOT NULL,
          public_inputs TEXT NOT NULL,
          FOREIGN KEY (user) REFERENCES users(id),
          FOREIGN KEY (sudoku) REFERENCES sudokus(id)
        )
      `, err => {
        if (err) return reject(err);
      });
      db.run(`
        CREATE TABLE IF NOT EXISTS verifications (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          verifier INTEGER,
          solution INTEGER,
          FOREIGN KEY (verifier) REFERENCES users(id),
          FOREIGN KEY (solution) REFERENCES resolved(id)
        )
      `, err => {
        if (err) return reject(err);
      });
      db.run(`
        CREATE TABLE IF NOT EXISTS blocks (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          user INTEGER,
          blocked_user INTEGER,
          FOREIGN KEY (user) REFERENCES users(id),
          FOREIGN KEY (blocked_user) REFERENCES users(id)
        )
      `, err => {
        if (err) return reject(err);
      });
    });
  });
}


module.exports = { db, initDatabase };