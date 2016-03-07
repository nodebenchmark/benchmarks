(function() {
  module.exports = function(app, settings) {
    var ObjectId, blog, fs, helper, media, user, util;
    blog = (require(__dirname + '/blog'))(settings);
    media = (require(__dirname + '/media'))(settings);
    helper = (require(__dirname + '/../helper'))();
    ObjectId = require('mongodb').ObjectID;
    util = require('util');
    if (process.env.NODE_ENV !== 'production') {
      fs = require('fs');
      user = require(__dirname + '/user')(settings);
      return fs.readFile(__dirname + '/../bin/post.md', 'utf8', function(err, result) {
        return blog["delete"](function() {
          var body, categories, category, content, imageUrl, post, posts, promise, _i, _j, _len, _len1, _ref, _ref1;
          posts = [];
          fs.readFile(__dirname + '/../public/logo.png', function(err, result) {
            var buffer, promise;
            buffer = result;
            if (Buffer.isBuffer(buffer)) {
              promise = media.create({
                id: new ObjectId(),
                slug: 'logo.png',
                type: 'image/png',
                body: buffer
              });
              return promise.then(function(result) {});
            }
          });
          _ref = result.split('#post');
          for (_i = 0, _len = _ref.length; _i < _len; _i++) {
            post = _ref[_i];
            if (post !== '') {
              content = post.split('#block');
              imageUrl = util.format('![Logo](http://localhost:%s/images/%s)', app.get('port'), helper.linkify('logo.png'));
              body = content[1].replace('#logo', imageUrl);
              categories = [];
              _ref1 = content[3].split(' ');
              for (_j = 0, _len1 = _ref1.length; _j < _len1; _j++) {
                category = _ref1[_j];
                category = category.replace(/^\n*|\n*$/g, '');
                categories.push(category);
              }
              promise = blog.createPost({
                title: content[0],
                body: body,
                author: settings.author,
                publish: true,
                categories: categories
              });
              promise.then(function(result) {
                if (result.id !== null) {
                  return console.log('[%s]', result.permaLink);
                }
              });
            }
          }
          return console.log('BootStrapping with data');
        });
      });
    }
  };

}).call(this);
