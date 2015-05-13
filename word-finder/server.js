var express = require('express');
var _ = require('underscore');
var app = express();

var config = require('./lib/config');
var words = require('./lib/words');

app.set('view engine', 'ejs');
app.set('view options', { layout: false });
app.use('/public', express.static('public'));

app.use(express.bodyParser());

app.use(app.router);

app.get('/', function (req, res) {
  res.render('index', { pattern: null });
});

app.post('/search', function (req, res) {
  var result = words.search(req.body.pattern).result;
  res.render('result', { words: result, pattern: req.body.pattern });
});

app.listen(process.env.PORT || config.port, config.host);
console.log("Listening on " + config.host + ":" + config.port);
console.log("BEGIN SENDING HTTP REQUESTS");
var fs = require('fs');
fs.writeFile("/tmp/word-finder", "meow!", function(err) {
    if(err) {
        return console.log(err);
    }
    console.log("The file was saved!");
}); 
