#!/usr/bin/env node

var minimist = require("minimist");

var Generator = require("./generator");
var Runner = require("./runner");

var FOLDER = "migrootions"

// Main Processing
// ---------------

var args = minimist(process.argv.slice(2));
var cwd = process.cwd();

if (args._.length > 0) {
  generateMigration(args);
} else {
  runMigrations();
}

function generateMigration(args){
  var migrationName = args._.join(" ");
  var generator = new Generator(cwd, FOLDER);

  generator.generate(migrationName, function(err, path){
    if (err) { throw err; }
    console.log("Generated migration at", path);
    process.exit();
  });
}

function runMigrations(){
  var runner = new Runner(cwd, FOLDER);
  runner.run(function(){
    process.exit();
  });
}
