(function() {
  var routes,
    _this = this;

  routes = function(app, settings) {
    var ObjectId, authorize, blog, category, helper, media, parseBody, parseCategories, processFeeds, request, util, xml2js;
    util = require('util');
    blog = (require(__dirname + '/modules/blog'))(settings);
    helper = (require(__dirname + '/helper'))();
    category = (require(__dirname + '/modules/category'))(settings);
    media = (require(__dirname + '/modules/media'))(settings);
    request = (require(__dirname + '/modules/request'))(settings);
    xml2js = require('xml2js');
    ObjectId = require('mongodb').ObjectID;
    authorize = function(req, res, next) {
      request.validate(req, function(result) {
        if (result !== null) {
          next();
        } else {
          res.header({
            'WWW-Authenticate': 'Basic realm="' + app.host + '"'
          });
          res.send(401);
        }
      });
    };
    parseBody = function(body) {
      var parser, promise, rawBody,
        _this = this;
      parser = new xml2js.Parser();
      body = new Buffer(body).toString();
      rawBody = body.replace(/atom:/ig, '');
      rawBody = rawBody.replace(/app:/ig, '');
      promise = new settings.Promise();
      parser.parseString(rawBody, function(err, result) {
        var content, entry, publish, title;
        entry = result.entry;
        publish = true;
        if (typeof entry.control !== 'undefined' && entry.control[0].draft[0] === 'yes') {
          publish = false;
        }
        title = entry.title[0];
        content = entry.content[0];
        if (typeof title._ !== 'undefined') {
          title = title._;
        }
        return promise.resolve({
          title: title,
          content: content._,
          publish: publish,
          categories: parseCategories(entry)
        });
      });
      return promise;
    };
    parseCategories = function(entry) {
      var cat, categories, _i, _len, _ref;
      categories = [];
      if (typeof entry.category !== 'undefined') {
        _ref = entry.category;
        for (_i = 0, _len = _ref.length; _i < _len; _i++) {
          cat = _ref[_i];
          categories.push(cat.$.term);
        }
      }
      return categories;
    };
    app.get('/api/atom', function(req, res) {
      res.header({
        'Content-Type': 'application/xml'
      });
      return res.render('atom/atom', {
        title: 'Blog entries',
        host: app.host
      });
    });
    app.get('/api/atom/categories', function(req, res) {
      res.header({
        'Content-Type': 'application/xml'
      });
      return category.all(function(result) {
        return res.render('atom/categories', {
          categories: result
        });
      });
    });
    processFeeds = function(req, res) {
      var format,
        _this = this;
      format = '';
      if (req.headers['accept'] && req.headers['accept'].indexOf('text/html') >= 0) {
        format = 'sanitize';
      }
      return request.validate(req, function(result) {
        promise;
        var promise;
        if (result !== null) {
          promise = blog.findAll(format);
        } else {
          promise = blog.find(format);
        }
        return promise.then(function(result) {
          res.header({
            'Content-Type': 'application/xml'
          });
          return res.render('atom/feeds', {
            host: app.host,
            title: result.title,
            updated: result.updated,
            posts: result.posts
          });
        });
      });
    };
    app.get('/api/atom/feeds', processFeeds);
    app.post('/api/atom/feeds', authorize, function(req, res) {
      var promise;
      promise = parseBody(req.rawBody);
      return promise.then(function(result) {
        var blogPromise;
        blogPromise = blog.createPost({
          title: result.title,
          body: result.content,
          publish: result.publish,
          author: settings.author,
          categories: result.categories
        });
        return blogPromise.then(function(result) {
          var location;
          location = app.host + 'api/atom/entries/' + result._id;
          res.header({
            'Content-Type': req.headers['content-type'],
            'Location': location
          });
          res.statusCode = 201;
          return res.render('atom/entries', {
            post: result,
            host: app.host
          });
        });
      });
    });
    app.get('/api/atom/entries/:id', function(req, res) {
      return blog.findPostById(req.params.id, function(result) {
        if (req.headers['accept'] && req.headers['accept'].indexOf('text/html') >= 0) {
          result.body = helper.htmlEscape(settings.format(result.body));
        }
        result.title = result.title.trim();
        res.header({
          'Content-Type': 'application/atom+xml'
        });
        return res.render('atom/entries', {
          post: result,
          host: app.host
        });
      });
    });
    app.put('/api/atom/entries/:id', authorize, function(req, res) {
      var promise;
      promise = parseBody(req.rawBody);
      return promise.then(function(result) {
        promise = blog.updatePost({
          id: req.params.id,
          title: result.title,
          body: result.content,
          publish: result.publish,
          categories: result.categories
        });
        return promise.then(function(result) {
          return res.render('atom/entries', {
            post: result,
            host: app.host
          });
        });
      });
    });
    app["delete"]('/api/atom/entries/:id', authorize, function(req, res) {
      return blog.deletePost(req.params.id, function() {
        return res.end();
      });
    });
    app.get('/rsd.xml', function(req, res) {
      res.header({
        'Content-Type': 'application/xml'
      });
      return res.render('rsd', {
        host: app.host,
        engine: settings.engine
      });
    });
    app.post('/api/atom/images', authorize, function(req, res) {
      var promise, slug;
      slug = req.headers['slug'];
      promise = media.create({
        id: new ObjectId(),
        slug: slug,
        type: req.headers['content-type'],
        body: req.rawBody
      });
      return promise.then(function(result) {
        res.statusCode = 201;
        return res.render('atom/media', {
          media: result,
          host: app.host,
          author: settings.author
        });
      });
    });
    app.get('/images/:year/:month/:slug', function(req, res) {
      var promise, url;
      url = util.format("%s/%s/%s", req.params.year, req.params.month, req.params.slug);
      promise = media.get(url);
      return promise.then(function(result) {
        if (result !== null) {
          return res.send(result.data);
        }
      }, function(err) {
        return res.send(404, err.message);
      });
    });
    app.get('/:year/:month/:title', function(req, res) {
      var link, promise, recent,
        _this = this;
      link = util.format("%s/%s/%s", req.params.year, req.params.month, req.params.title);
      recent = [];
      promise = blog.findMostRecent();
      return promise.then(function(result) {
        recent = result;
        promise = blog.findPost(link);
        return promise.then(function(result) {
          if (result !== null) {
            result.host = app.host;
            result.recent = recent;
            return res.render('post', result);
          } else {
            promise = blog.hasPostMoved(link);
            return promise.then(function(result) {
              if (result !== null) {
                return res.redirect(301, "/" + JSON.parse(result.content).permaLink);
              } else {
                return res.end("Invalid url or could not find the post");
              }
            });
          }
        });
      });
    });
    return app.get('/', function(req, res) {
      var promise, recent,
        _this = this;
      recent = [];
      promise = blog.findMostRecent();
      return promise.then(function(result) {
        recent = result;
        promise = blog.find('list');
        return promise.then(function(result) {
          result.host = app.host;
          result.recent = recent;
          return res.render('index', result);
        });
      });
    });
  };

  module.exports = routes;

}).call(this);
