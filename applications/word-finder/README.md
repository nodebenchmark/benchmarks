##Word Finder (MIT License)

I took every english word (over 200k words) and built a little NodeJS app that will help you find words that contain specific characters.

Additionally, here are instructions to deploy this app to Nodejitsu, Heroku, and Azure via Windows or Mac.

##How to Use

###The underscore

Type a word into the text box with the following pattern:

    he__o

And you'll get words such as:

    hello
    helio

###The question mark

This character is great for games like What's the Phrase (a knock off of Wheel of Fortune)

Type a word into the text box with the following pattern:

    st???

and you'll get words such as:

    stack
    stade
    staff
    stage
    stagy

but you wont get words like

    start

because the `t` would already be visible (in What's the Phrase), and you would have typed:

    st??t

##Instructions for running

Go to http://nodejs.org and install NodeJS

Then clone this repo:

    git clone https://github.com/amirrajan/word-finder.git

And `cd` into the directory (all instructions below assume you are in the `word-finder` directory:

    cd word-finder

##Run Locally

Install all the dependencies:

    npm install (you may need to prefix this with sudo if you're on Mac)

To run tests, type:

    jasmine-node .

If you want tests to execute every time you change a file:

    jasmine-node . --autotest --watch .

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
      "name": "word-finder",
      [...],
      "subdomain": "word-finder" <--- this value
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

