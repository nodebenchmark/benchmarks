##Todo App (MIT License)

Simple redis backed todo app.

##Install Node and Redis

Go to http://nodejs.org and install NodeJS

Go to http://redis.io/download and install Redis

##Run Locally

Install all the dependencies:

    npm install (you may need to prefix this with sudo if you're on Mac)

Run the app:

    node server.js

Then navigate to `http://localhost:3000`

##Signing up, and deploying to Nodejitsu

###Documentation

The documenation was available on the front page (right under the sign up for free button): https://www.nodejitsu.com/getting-started/

Install the Nodejitsu Package

    npm install jitsu -g (you may need to prefix this with sudo if you're on Mac)

Register via the command line:

    jitsu signup (yes you can sign up via the command line)

You'll get a confirmation email with a command to type in:

    jitsu users confirm [username] [confirmation-guid]

If you've already registered, you can login with:

    jitsu login

After you confirm your email, you can login (the `confirm` command should prompt you to log in).

Change the `subdomain` value in `package.json`, to reflect the url you want to deploy to:

    {
      "name": "nodejs-todo",
      [...],
      "subdomain": "nodejs-todo" <--- this value
    }

create a redis database:

    jitsu databases create redis todo

You'll get an output similar to this:

    info: Executing command databases create redis todo
    info: A new redis has been created
    data: Database Type: redis
    data: Database Name: todo
    data: Connection host: nodejitsu___________.redis._______.com
    data: Connection port: 6379
    data: Connection auth: nodejitsu___________.redis._______.com:__________

update the values in `secret.js`

    module.exports = {
        ...
        "redisPort": 6379, //Connection port value from output above
        "redisMachine": "", //Connection host value from output above
        "redisAuth": "", //Connection auth value from output above
        ...
    };

now deploy:

    jitsu deploy

note: **if you add lib/secret.js to your .gitignore it will not be deployed and the app will not run**. Ideally (once you get the hang of deploying this app), you'll want to move all the information in secret.js to environment variables in your production environment, for information on getting and setting environment variables for nodejitsu use `jitsu help env`.

Here is what secret.js may look like after migrating everything over to environment variables:

    module.exports = {
        "consumerKey": process.env.consumerKey,
        [...]
    }

And your app should be up on Nodejitsu.

##Signing up, and deploying to Heroku

###Documentation

From heroku.com, click Documentation, then click the Getting Started button, then click Node.js from the list of options on the left...which will take you here: https://devcenter.heroku.com/articles/nodejs 

Install Heroku toolbelt from here: https://toolbelt.heroku.com/

Sign up via the website (no credit card required).

Login using the command line tool:

    heroku login

Create your heroku app:

    heroku create

Add redis to your app

    heroku addons:add redistogo:nano

For heroku, the `redisPort`, `redisMachine`, `redisAuth` values in `secret.js` are not used (the Redis connection in Heroku is provided by an enviornment variable `process.env.REDISTOGO_URL`

Git deploy your app:

    git push heroku master

note: **if you add lib/secret.js to your .gitignore it will not be deployed and the app will not run**. Ideally (once you get the hang of deploying this app), you'll want to move all the information in secret.js to environment variables in your production environment, for information on getting and setting environment variables for heroku use `heroku help config`

Here is what secret.js may look like after migrating everything over to environment variables:

    module.exports = {
        "consumerKey": process.env.consumerKey,
        [...]
    }

Assign a dyno to your app:

    heroku ps:scale web=1

Open the app (same as opening it in the browser):

    heroku open

And your app should be up on Heroku.

##Signing up, and deploying to Azure
Azure does not offer a PaaS offering for NodeJS + Redis at this time
