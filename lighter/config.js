(function() {
  var config, express, util;

  express = require('express');

  util = require('util');

  config = function(app) {
    app.set('port', process.env.PORT || 3000);
    app.set('views', __dirname + '/views');
    app.set('view engine', 'jade');
    app.use(express.favicon(__dirname + '/public/favicon.ico'));
    app.use(express.logger('dev'));
    app.use(function(req, res, next) {
      var data, length,
        _this = this;
      app.host = util.format('http://%s/', req.headers['host']);
      data = [];
      length = 0;
      req.on('data', function(chunk) {
        length += chunk.length;
        data.push(chunk);
      });
      return req.on('end', function() {
        req.rawBody = Buffer.concat(data, length);
        next();
      });
    });
    app.use(app.router);
    app.use(express.urlencoded());
    app.use(express.json());
    app.use(express.methodOverride());
    app.use(require('less-middleware')({
      src: __dirname + '/public'
    }));
    app.use(express.compress());
    if (process.env.NODE_ENV === 'production') {
      app.use(express["static"](__dirname + '/public', {
        maxAge: 86400000
      }));
    } else {
      app.use(express["static"](__dirname + '/public'));
    }
    app.locals.pretty = true;
    if ('development' === app.get('env')) {
      return app.use(express.errorHandler());
    }
  };

  module.exports = config;

}).call(this);
