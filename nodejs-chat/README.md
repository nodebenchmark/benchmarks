##NodeJS Chat (MIT License)

A node js chat app using web sockets (socket.io)

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
      "name": "nodejs-chat",
      [...],
      "subdomain": "nodejs-chat" <--- this value
    }

now deploy:

    jitsu deploy

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

Git deploy your app:

    git push heroku master

Assign a dyno to your app:

    heroku ps:scale web=1

Open the app (same as opening it in the browser):

    heroku open

And your app should be up on Heroku.

##Signing up, and deploying to Azure

###Documentation

From windowsazure.com, click Documentation, click Developer Center, click node.js, then click the Learn More button which will take you here:

http://www.windowsazure.com/en-us/develop/nodejs/tutorials/create-a-website-(mac)/ (if you're on a Mac, looks like the link is contextual)

Install the command line tools from here:

http://www.windowsazure.com/en-us/downloads/#cmd-line-tools (on Windows, be sure to install the cross platform command line interface...not the powershell version)

From the command line, first download your publish settings (this will redirect you to a website):

    azure account download

After the `.publishsettings` file is downloaded, you'll need to import it:

    azure acount import %pathtofile%

Next create the site, with a git backed repository:
    
    azure site create %uniquesitename% --git

Deploy site:

    git push azure master

List of your websites:

    azure site list

And your app should be up on Azure.
