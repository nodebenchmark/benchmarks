# Migroose-CLI

Command line tooling for the [migroose migration framework](https://github.com/derickbailey/migroose).

## Get Started

You will need both [migroose](https://github.com/derickbailey/migroose) and
the migroose-cli tool. Migroose should be installed in the
project, while the cli tool should be installed globally.

```
npm install migroose
npm install -g migroose-cli
```

Now you can use the cli tool to generate a migration and
run migrations.

### Generate A Migration

Use the command line to generate a migration file.

```
migroose "some example migration"
```

This will produce a `mongrations/##########-some-example-migration.js` file
where "#########" is a timestamp.

See the [migroose docmentation](https://github.com/derickbailey/migroose) for information on how
to write a migroose migration script. 

### Connect Migroose To Your MongoDB Database

Before you can run you migrations, you need to provide a
connection to your MongoDB database. This only has to be done
once per project, but it must be done before the migrations 
can run.

Create a `migroose.js` file in your project folder, and
have it export a `connect` function. This function receives
one callback argument that you must call once your database
connection has been established.

```js
// my-project/migroose.js

var mongoose = require("mongoose");

module.exports = {

  // provide a connection to my mongodb instance
  connect: function(cb){

    var conn = "mongodb://localhost:27017/some-database";
    mongoose.connect(conn, function(err){
      if (err) { throw err; }

      // now that i'm connected, i can tell migroose to run
      cb();
    });

  }
};
```

Having written this connector, you can now use the migroose
command line to run your migrations.

### Run Migrations

Once you have completed your migraiton script, you can run them with the command
line.

```
migroose
```

Specifying no parameters will tell migroose to run all of the migration files
found in the `mongrations` folder.

### Legal Junk

Migroose-CLI is &copy;Copyright 2015, Muted Solutions LLC.

Migroose and Migroose-CLI are distributable under the [MIT License](http://mutedsolutions.mit-license.org)
