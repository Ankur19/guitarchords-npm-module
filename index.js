//index.js
//const testAddon = require("./build/Release/guitarchords.node");
const testAddon = require("./build/Release/guitarchords.node");

let notes = ["E", "A", "D", "G", "B", "E"];
let tuning = new testAddon.Tuning(...notes);

let x = testAddon.getPositions(tuning, "D", "MAJOR", 1, 3);
console.log(x);
