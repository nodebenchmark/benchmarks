# Mongoose Validate

  Additional Validation functions for your mongoose schema.

  [![Build Status](https://secure.travis-ci.org/RGBboy/mongoose-validate.png)](http://travis-ci.org/RGBboy/mongoose-validate)

## Installation

  Works with Mongoose 3.3.x

    $ npm install git://github.com/RGBboy/mongoose-validate.git

## Usage

Mongoose Validate has been written to be used directly when declaring a Mongoose Schema:

``` javascript
var validate = require('mongoose-validate')
  , mongoose = require('mongoose')
  , Schema = mongoose.Schema
  , NewSchema;

NewSchema = new Schema({
  email: { type: String, required: true, validate: [validate.email, 'invalid email address'] }
});

```

### .email

Validates an email.

### .alpha

Validates any alpha character (a-z, A-Z).

### .alphanumeric

Validates any alphanumeric character (a-z, A-Z, 0-9).

### .numeric

Validates any numeric character (0-9).

### .postalCode

Validates a postal code. The validation accepts any alphanumeric string with a single hyphen
or space that is at least 3 characters in length and no more than 10 characters in length.
The valid postal code format was found at http://en.wikipedia.org/wiki/Postal_code

### .permalink

Validates a permalink. The validation accepts any lowercase alphanumeric characters and hyphens.

## To Do

  * Write tests;
  * Add other validation;
