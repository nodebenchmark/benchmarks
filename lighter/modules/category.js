(function() {
  module.exports = function(settings) {
    var Category;
    Category = (function() {
      function Category(settings) {
        this.category = settings.mongoose.model('category');
        this.helper = (require(__dirname + '/../helper'))();
        this.settings = settings;
        this.cats = [];
      }

      Category.prototype.refresh = function(category) {
        var link, promise, title,
          _this = this;
        promise = new this.settings.Promise;
        title = category.trim();
        link = this.helper.linkify(title);
        this.category.findOne({
          title: title
        }, function(err, data) {
          if (data === null) {
            category = new _this.category({
              title: title,
              permaLink: link
            });
            return category.save(function(err, data) {
              return promise.resolve(data);
            });
          } else {
            return promise.resolve(data);
          }
        });
        return promise;
      };

      Category.prototype.all = function(callback) {
        return this.category.find(function(err, data) {
          return callback(data);
        });
      };

      Category.prototype.clear = function(callback) {
        return this.category.remove(function() {
          return callback();
        });
      };

      return Category;

    })();
    return new Category(settings);
  };

}).call(this);
