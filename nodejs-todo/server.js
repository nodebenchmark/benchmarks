var express = require('express');
var _ = require('underscore');
var app = express();
var server = require('http').createServer(app);
var parseCookie = require('./lib/cookie_parser');
var config = require('./lib/config');
var redis = require("redis");
var secret = require('./lib/secret');

var client = null;
if(process.env.REDISTOGO_URL) { //heroku
  client = require('redis-url').connect(process.env.REDISTOGO_URL); 
} else if(config.env == "development") {
  client = redis.createClient();
}  else { //nodejitsu
  client = redis.createClient(secret.redisPort, secret.redisMachine);
  client.auth(secret.redisAuth, function (err) {
     if (err) { throw err; }
  });
}

app.set('view engine', 'ejs');
app.set('view options', { layout: false });
app.use('/public', express.static('public'));

app.use(express.methodOverride());
app.use(express.bodyParser());
app.use(express.cookieParser());
app.use(express.session({secret: guid()}));

app.use(app.router);

//helper method for writing out json payloads
var json = function(res, data) {
  res.writeHead(200, { 'Content-Type': 'application/json; charset=utf-8' });

  if(typeof data === "string") res.write(data);

  else res.write(JSON.stringify(data));

  res.end();
};

function guid() {
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
      var r = Math.random() * 16|0, v = c == 'x' ? r : (r&0x3|0x8);
      return v.toString(16);
  });
}

app.get('/', function (req, res) {
  res.render('index');
});

app.get('/todos', function(req, res) {
  client.hgetall("todos", function(err, data) {
    json(res, data);
  });
});

app.post('/todos/create', function(req, res) {
  var id = guid();
  client.hset("todos", id, req.body.description);

  json(res, { id: id });
});

app.post('/todos/update', function(req, res) {
  client.hset("todos", req.body.id, req.body.description);

  json(res, { });
});

app.post('/todos/delete', function(req, res) {
  client.hdel("todos", req.body.id);

  json(res, { });
});

server.listen(process.env.PORT || config.port);
