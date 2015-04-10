(function() {
  module.exports = function(settings) {
    var User;
    User = (function() {
      function User(settings) {
        this.settings = settings;
        this.user = settings.mongoose.model('user');
        this.crypto = require('crypto');
      }

      User.prototype.init = function(callback) {
        var _this = this;
        return this.user.findOne({
          username: settings.username
        }, function(err, data) {
          var password, user;
          password = _this.crypto.createHash('md5').update(settings.password.trim()).digest('hex');
          if (data === null) {
            user = new _this.user({
              username: settings.username,
              password: password,
              active: true,
              created: new Date()
            });
            return user.save(function(err, data) {
              if (err !== null) {
                throw err;
              }
              return callback(data);
            });
          } else {
            data.username = settings.username;
            data.password = password;
            return data.save(function(err, data) {
              return callback(data);
            });
          }
        });
      };

      User.prototype.find = function(username, password, callback) {
        password = password.trim();
        password = this.crypto.createHash('md5').update(password).digest('hex');
        return this.user.findOne({
          username: username.trim(),
          password: password
        }, function(err, data) {
          return callback(data);
        });
      };

      User.prototype["delete"] = function(callback) {
        return this.user.remove(function() {
          return callback();
        });
      };

      return User;

    })();
    return new User(settings);
  };

}).call(this);
