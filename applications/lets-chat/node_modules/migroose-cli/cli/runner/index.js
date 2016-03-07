var fs = require("fs");
var path = require("path");

// Migration Runner
// ----------------

function Runner(cwd, migrationFolder){
  this.cwd = cwd;
  this.migrationFolder = migrationFolder;
}

// Instance Methods
// ----------------

Runner.prototype.run = function(cb){
  var that = this;

  var migrations = this.getMigrations();
  if (migrations.length === 0){ return; }

  this.openConnection(function(){
    that.doMigrations(migrations, cb);
  });
};

// Private Methods
// ---------------

Runner.prototype.getMigrations = function(){
  var cwd = this.cwd;
  var migrations = [];

  var folder = path.join(this.cwd, this.migrationFolder);
  fs.readdirSync(folder).forEach(function(file){
    if (!isJSFile(file)){ return; }

    var migrationFile = path.join(folder, file);
    var migration = require(migrationFile);
    migrations.push(migration);
  });

  return migrations;
};

Runner.prototype.openConnection = function(cb){
  var migrooseFile = path.join(this.cwd, "migroose.js");
  var connector = require(migrooseFile);
  connector.connect(cb);
};

Runner.prototype.doMigrations = function(migrations, cb){
  var that = this;

  var migration = migrations.shift();
  if (!migration) { return cb(); }

  migration.migrate(function(err){
    if (err) { throw err; }

    setImmediate(function(){
      that.doMigrations(migrations, cb);
    });
  });
};

// Helpers
// -------

function isJSFile(file){
  var index = file.indexOf(".js");
  var location = file.length - 3;
  return (index === location)
}

// Exports
// -------

module.exports = Runner;
