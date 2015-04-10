(function() {
  var __bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; };

  module.exports = function(settings) {
    var Blog;
    Blog = (function() {
      function Blog(settings) {
        this.createPost = __bind(this.createPost, this);
        var path;
        this.settings = settings;
        path = require('path');
        this.blog = settings.mongoose.model('blog');
        this.post = settings.mongoose.model('post');
        this.helper = (require(__dirname + '/../helper'))();
        this.category = (require(__dirname + '/category'))(settings);
        this.media = (require(__dirname + '/media'))(settings);
        this.map = settings.mongoose.model('map');
      }

      Blog.prototype.create = function() {
        var promise,
          _this = this;
        promise = new this.settings.Promise();
        this.blog.findOne({
          url: this.settings.url
        }, function(err, data) {
          var blog;
          if (data !== null) {
            if (data.title !== _this.settings.title) {
              data.title = _this.settings.title;
              return data.save(function(err, data) {
                return promise.resolve(data);
              });
            } else {
              return promise.resolve(data);
            }
          } else {
            blog = new _this.blog({
              url: _this.settings.url,
              title: _this.settings.title,
              updated: _this.settings.updated
            });
            return blog.save(function(err, data) {
              return promise.resolve(data);
            });
          }
        });
        return promise;
      };

      Blog.prototype.createPost = function(obj) {
        var promise,
          _this = this;
        promise = new this.settings.Promise();
        this.create().then(function(result) {
          if (result !== null) {
            return _this._post({
              id: result._id,
              post: obj
            }, function(data) {
              return promise.resolve(data);
            });
          }
        });
        return promise;
      };

      Blog.prototype.find = function(format, filter) {
        var promise,
          _this = this;
        promise = new this.settings.Promise();
        this.blog.findOne({
          url: this.settings.url
        }, function(err, data) {
          var blog;
          if (err !== null) {
            throw err.message;
          }
          blog = data;
          if (typeof filter === 'undefined') {
            filter = {
              id: blog._id,
              publish: true
            };
          } else {
            filter.id = blog._id;
          }
          return _this.post.find(filter).sort({
            date: -1
          }).exec(function(err, data) {
            var post, posts, _i, _len;
            posts = [];
            for (_i = 0, _len = data.length; _i < _len; _i++) {
              post = data[_i];
              if (format === 'sanitize' || 'list') {
                post.body = _this.settings.format(post.body);
                if (format === 'list') {
                  post.body = post.body.match(/<p>(.*?)<\/p>/g)[0];
                }
              }
              post.title = post.title.trim();
              posts.push(post);
            }
            return promise.resolve({
              id: blog._id,
              title: blog.title,
              updated: blog.updated,
              posts: posts
            });
          });
        });
        return promise;
      };

      Blog.prototype.findAll = function(format) {
        return this.find(format, {});
      };

      Blog.prototype.findMostRecent = function() {
        var promise,
          _this = this;
        promise = new this.settings.Promise;
        this.blog.findOne({
          url: this.settings.url
        }, function(err, data) {
          return _this.post.find({
            id: data._id,
            publish: true
          }).sort({
            date: -1
          }).limit(5).exec(function(err, data) {
            var post, recent, _i, _len;
            recent = [];
            for (_i = 0, _len = data.length; _i < _len; _i++) {
              post = data[_i];
              recent.push({
                title: post.title,
                permaLink: post.permaLink
              });
            }
            return promise.resolve(recent);
          });
        });
        return promise;
      };

      Blog.prototype.findPost = function(permaLink) {
        var promise,
          _this = this;
        promise = new this.settings.Promise;
        this.blog.findOne({
          url: this.settings.url
        }, function(err, data) {
          var blog;
          blog = data;
          return _this.post.findOne({
            id: blog._id,
            permaLink: permaLink
          }, function(err, data) {
            var summary;
            if (err !== null || data === null) {
              promise.resolve(null);
              return;
            }
            summary = data.body.trim();
            data.summary = summary.substring(0, summary.indexOf('\n'));
            data.body = _this.settings.format(data.body);
            return promise.resolve({
              title: blog.title,
              post: data
            });
          });
        });
        return promise;
      };

      Blog.prototype.hasPostMoved = function(permaLink) {
        var promise;
        promise = new this.settings.Promise;
        this.map.findOne({
          permaLink: permaLink
        }, function(err, data) {
          return promise.resolve(data);
        });
        return promise;
      };

      Blog.prototype.findPostById = function(id, callback) {
        var _this = this;
        return this.post.findOne({
          _id: id
        }, function(err, data) {
          return callback(data);
        });
      };

      Blog.prototype.updatePost = function(post) {
        var promise,
          _this = this;
        promise = new this.settings.Promise;
        this.post.findOne({
          _id: post.id
        }, function(err, data) {
          var category, previous, _i, _len, _ref;
          previous = {
            id: data._id,
            title: data.title,
            permaLink: data.permaLink,
            body: data.body
          };
          data.body = post.body;
          if (data.title !== post.title) {
            data.title = post.title;
            data.permaLink = _this.helper.linkify(post.title);
          }
          data.categories = post.categories;
          data.publish = post.publish;
          if (data.categories) {
            _ref = data.categories;
            for (_i = 0, _len = _ref.length; _i < _len; _i++) {
              category = _ref[_i];
              _this.category.refresh(category);
            }
          }
          return data.save(function(err, data) {
            var permaLink;
            post = data;
            permaLink = previous.permaLink;
            return _this.map.findOne({
              permaLink: permaLink
            }, function(err, data) {
              var map;
              if (data === null) {
                map = new _this.map;
                map.permaLink = permaLink;
              } else {
                map = data;
              }
              map.content = JSON.stringify({
                id: post.id,
                title: post.title,
                permaLink: post.permaLink,
                body: post.body
              });
              return map.save(function(err, data) {
                if (err === null) {
                  return promise.resolve(post);
                } else {
                  throw err;
                }
              });
            });
          });
        });
        return promise;
      };

      Blog.prototype.deletePost = function(id, callback) {
        var _this = this;
        return this.post.remove({
          _id: id
        }, function() {
          _this.map.remove(function() {});
          return callback();
        });
      };

      Blog.prototype["delete"] = function(callback) {
        var _this = this;
        this.blog.find({
          url: this.settings.url
        }, function(err, data) {
          var blog, _i, _len, _results;
          _results = [];
          for (_i = 0, _len = data.length; _i < _len; _i++) {
            blog = data[_i];
            _results.push(_this.post.remove({
              id: blog._id
            }, function() {
              return _this.blog.remove({
                url: _this.settings.url
              });
            }));
          }
          return _results;
        });
        this.category.clear(function() {});
        this.media.clear(function() {});
        return this.map.remove(function() {
          return callback();
        });
      };

      Blog.prototype._post = function(obj, callback) {
        var permaLink, post, postSchema,
          _this = this;
        post = obj.post;
        permaLink = this.helper.linkify(post.title);
        if (typeof post.permaLink !== 'undefined') {
          permaLink = post.permaLink;
        }
        postSchema = new this.post({
          id: obj.id,
          title: post.title,
          permaLink: permaLink,
          author: post.author,
          body: post.body,
          publish: post.publish,
          date: new Date(),
          categories: post.categories
        });
        return postSchema.save(function(err, data) {
          var category, _i, _len, _ref;
          if (err !== null) {
            callback(err.message);
          }
          if (data.categories) {
            _ref = data.categories;
            for (_i = 0, _len = _ref.length; _i < _len; _i++) {
              category = _ref[_i];
              _this.category.refresh(category, function(id) {});
            }
          }
          callback(data);
        });
      };

      return Blog;

    })();
    return new Blog(settings);
  };

}).call(this);
