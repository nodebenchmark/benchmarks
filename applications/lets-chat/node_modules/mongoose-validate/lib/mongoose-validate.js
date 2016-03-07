/*!
 * Mongoose Validate
 */

/**
 * Module Dependencies
 */

var check = require('validator').check,
    validate = {};

/**
* Library version.
*/

validate.version = '0.0.5';

/**
 * .email
 *
 * @param {String} value
 *
 * @api public
 */
validate.email = function (value) {
  try {
    check(value).isEmail();
  } catch(err) {
    return false;
  }
  return true;
};

/**
 * .address
 *
 * @todo: Add address validation
 *        firstName
 *        lastName
 *        organisation
 *        address1..4
 *        suburb
 *        city
 *        country
 */

// Check out validator.checks validation

/**
 * .alpha
 *
 * @param {String} value
 *
 * @api public
 */
validate.alpha = function (value) {
  try {
    check(value).isAlpha();
  } catch(err) {
    return false;
  }
  return true;
};

/**
 * .alphanumeric
 *
 * @param {String|Number} value
 *
 * @api public
 */
validate.alphanumeric = function (value) {
  try {
    check(value).isAlphanumeric();
  } catch(err) {
    return false;
  }
  return true;
};

/**
 * .numeric
 *
 * @param {String|Number} value
 *
 * @api public
 */
validate.numeric = function (value) {
  try {
    check(value).isNumeric();
  } catch(err) {
    return false;
  }
  return true;
};

/**
 * .int
 *
 * @param {String|Number} value
 *
 * @api public
 */
validate.int = function (value) {
  try {
    check(value).isInt();
  } catch(err) {
    return false;
  }
  return true;
};

/**
 * .postalCode
 *
 * Postal Codes should be between 3 and 10 alphanumeric characters according
 * to http://en.wikipedia.org/wiki/Postal_code
 *
 * @param {String|Number} value
 *
 * @api public
 */
validate.postalCode = function (value) {
  try {
    check(value).len(3, 10).regex(/^[a-zA-Z0-9]+[ -]{0,1}[a-zA-Z0-9]+$/);
  } catch(err) {
    return false;
  }
  return true;
};

/**
 * .permalink
 *
 * @param {String|Number} value
 *
 * @api public
 */
validate.permalink = function (value) {
  try {
    check(value).regex(/^[a-z0-9\-]+$/);
  } catch(err) {
    return false;
  }
  return true;
};


/**
 * Module Exports
 */

module.exports = exports = validate;

