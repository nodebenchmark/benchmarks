var fs = require("fs");
var path = require("path");

// Migration Generator
// -------------------

function Generator(cwd, migrationFolder){
  this.cwd = cwd;
  this.migrationFolder = migrationFolder;
}

// Instance Methods
// ----------------

Generator.prototype.generate = function(name, cb){
  var config = this.getConfig(name);
  var content = this.generateFileContent(config);
  this.writeFile(content, config, cb);
};

// Private Methods
// ---------------

Generator.prototype.getConfig = function(name){
  var timestamp = Date.now();
  var slug = getSlug(name);
  var descriptor = timestamp.toString() + "-" + slug;
  var filename = descriptor + ".js";
  var folderPath = path.join(this.cwd, this.migrationFolder);
  var fullPath = path.join(folderPath, filename);

  var config = {
    timestamp: timestamp,
    slug: slug,
    descriptor: descriptor,
    filename: filename,
    folderPath: folderPath,
    fullPath: fullPath
  };

  return config;
};

Generator.prototype.generateFileContent = function(config){
  var content = "";

  content += "var Migroose = require('migroose');\r\n";
  content += "var migration = new Migroose.Migration('" + config.descriptor + "');\r\n\r\n";
  content += "migration.step(function(data, stepComplete){\r\n";
  content += "  console.log('do some work');\r\n";
  content += "  stepComplete();\r\n";
  content += "});\r\n\r\n";
  content += "module.exports = migration;";

  return content;
};

Generator.prototype.writeFile = function(content, config, cb){
  var exists = fs.existsSync(config.folderPath);
  if (!exists) {
    fs.mkdirSync(config.folderPath);
  }
  
  fs.writeFile(config.fullPath, content, function(err) {
    if (err) { return cb(err); }
    cb(null, config.fullPath);
  }); 
};

// Helpers
// -------

function getSlug(str){
  return str.replace(/[^0-9a-zA-Z\.\-]+/g, "-");
}

// Exports
// -------

module.exports = Generator;
