#lighter - fast, responsive blog engine. 
     
The blog is built using node and mongo. There is no admin view and exposes AtomPub to create, view and delete posts.

[![Build Status](https://travis-ci.org/mehfuzh/lighter.png)](https://travis-ci.org/mehfuzh/lighter)

##Installation

The quickest way to get started  with _lighter_ is to run the following command once you downloaded the source:

	npm install

This will install the necessary dependencies for running the project. Once done just start the server by executing the following command:

	npm start

Blog modules are written with coffee script and automatically compiled in runtime using a coffescript middleware.You will also need mongodb. Please refer to the following online document for installing mongo locally:                            

[Installing MongoDB] (http://docs.mongodb.org/manual/installation/)

Optionally, use the following command to install the MongoDB package into your system:

	brew install mongodb

##Configuration

The project structure is similar to the default express web template. Stylesheets are written with less and project settings are defined in _settings.coffee_ under modules folder.

You don't have to change the settings manually rather use config variables to do so. If you are using cloud services then most cases it can be done using shell. For example if you are using heroku you can do this:

	heroku config set:BLOG_TITLE=Awesome Blog

If you are using other cloud services then please check their documentation for further information. 

List of config variables that you can use to modify the default settings:

+	BLOG_TITLE: Title of the blog. (default: Lighter Blog)
+	AUTHOR: Main author of the blog. (default: Editor).
+	USER: Admin username. (default: your process username)
+	PASSWORD: Admin password. (default: admin)
+	FEED_URL: Feedbunder url mapped with your blog feed. The private url that you will be setting in feedburner is <yourdomain>/atom/feeds

You don't have to set the __MONGOLAB_URI / MONGO_URI__ manually since this is set automatically by cloud services. If you have mongo installed locally then you dont have to change it either unless you changed the default port.

In addition, please set _NODE_ENV=production_ as you deploy your blog. By default the blog is refreshed with dummy posts each time the blog is deployed unless you set this varaible. 

The blog uses _newrelic_ for monitoring. All the variables are set automtically uneless otherwise mentioned by your provider.

##Running Tests

Tests are written with mocha. It is required that mocha is installed globally:

	npm install -g mocha


Once installed, type the following command to run tests.

	npm test

##Further Reading

My personal blog is hosted with *lighter* @www.meonbinary.com. 

Please check this post out for further reading:            
[Introducing Lighter Blog Engine](http://www.meonbinary.com/2013/02/introducing-lighter-blog-engine)

##Key Facts
The blog does not have an admin panel. It exposes ATOM Pub and I use __MarsEdit__ to view/edit/delete post(s).
                      
##License 

Copyright © 2013 Mehfuz Hossain

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.