(function() {
  module.exports = function(settings) {
    var Request;
    Request = (function() {
      function Request(settings) {
        this.settings = settings;
        this.user = require(__dirname + '/user')(settings);
      }

      Request.prototype.validate = function(req, callback) {
        var authValue, buff, content, credentials, password, username;
        authValue = req.headers['authorization'];
        if (typeof authValue !== 'undefined' && authValue.indexOf('Basic') >= 0) {
          buff = new Buffer(authValue.split(' ')[1], 'base64');
          content = buff.toString('utf8');
          if (content.indexOf(':') >= 0) {
            credentials = content.split(':');
            username = credentials[0];
            password = credentials[1];
            this.user.find(username, password, function(data) {
              callback(data);
            });
          }
        } else {
          return callback(null);
        }
      };

      return Request;

    })();
    return new Request(settings);
  };

}).call(this);
