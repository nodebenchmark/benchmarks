var path = require('path');
var fs = require('fs');

var env = process.env['NODE_ENV'] || 'development';
var json_path = path.join(__dirname, '../config', env + '.json');

var config = JSON.parse(fs.readFileSync(json_path));

config.env = env;

module.exports = config;
