(function() {
  module.exports = function() {
    var Helper, util;
    util = require('util');
    Helper = (function() {
      function Helper() {}

      Helper.prototype.linkify = function(source) {
        source = source.toLowerCase();
        source = source.replace(/^\s*|\s*$/g, '');
        source = source.replace(/:+/g, '');
        source = source.replace(/[?#&!()+=]+/g, '');
        source = source.replace(/\s+/g, '-');
        return util.format("%s/%s", this.dateNow(), source);
      };

      Helper.prototype.dateNow = function() {
        var dateNow, mm, yy;
        dateNow = new Date();
        mm = dateNow.getMonth() + 1;
        if (mm < 10) {
          mm = '0' + mm;
        }
        yy = dateNow.getFullYear();
        return util.format('%s/%s', yy, mm);
      };

      Helper.prototype.htmlEscape = function(html) {
        html = html.replace(/&(?!\w+;)/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
        return html;
      };

      return Helper;

    })();
    return new Helper();
  };

}).call(this);
