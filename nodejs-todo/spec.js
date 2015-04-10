var jasmine = require('jasmine-node');

for(var key in jasmine) {
  global[key] = jasmine[key];
}

var isVerbose = true;
var showColors = true;

process.argv.forEach(function(arg){
    switch(arg) {
          case '--color': showColors = true; break;
          case '--noColor': showColors = false; break;
          case '--verbose': isVerbose = true; break;
      }
});

jasmine.executeSpecsInFolder(__dirname + '/specifications', function(runner, log){
    process.exit(0);
}, isVerbose, showColors);

