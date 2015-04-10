module.exports = function parseCookie(str) {
  var obj = {}
  var pairs = str.split(/[;,] */);
  var encode = encodeURIComponent;
  var decode = decodeURIComponent;

  pairs.forEach(function(pair) {
    var eq_idx = pair.indexOf('=')
    var key = pair.substr(0, eq_idx).trim()
    var val = pair.substr(++eq_idx, pair.length).trim();

    // quoted values
    if ('"' == val[0]) {
      val = val.slice(1, -1);
    }

    // only assign once
    if (undefined == obj[key]) {
      obj[key] = decode(val);
    }
  });

  return obj;
};
