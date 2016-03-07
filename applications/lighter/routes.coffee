routes = (app, settings) => 
	util = require('util')
	blog = (require __dirname+'/modules/blog')(settings) 
	helper = (require __dirname+'/helper')()
	
	category = (require __dirname + '/modules/category')(settings) 
	media = (require __dirname + '/modules/media')(settings) 
	
	request = (require __dirname + '/modules/request')(settings)
	xml2js = require 'xml2js'
	ObjectId = require('mongodb').ObjectID

	authorize = (req, res, next)->
		request.validate req, (result)->
			if result isnt null
				next()
				return
			else
				res.header({'WWW-Authenticate': 'Basic realm="' + app.host + '"'})
				res.send(401) 
				return
		return

	parseBody = (body)->
		parser = new xml2js.Parser()
		body = new Buffer(body).toString()

		rawBody = body.replace(/atom:/ig, '')
		rawBody = rawBody.replace(/app:/ig, '')

		promise = new settings.Promise()

		parser.parseString rawBody, (err, result) => 
			entry = result.entry			

			publish = true

			if typeof entry.control isnt 'undefined' && entry.control[0].draft[0] == 'yes'
				publish = false

			title = entry.title[0]
			content = entry.content[0]

			if typeof title._ isnt 'undefined'
				title = title._
	
			promise.resolve 
				title 		: title
				content 	: content._	
				publish	: publish
				categories 	: parseCategories(entry)

		return promise			
		
	parseCategories = (entry)->
		categories = []
		# process category.
		if typeof(entry.category) != 'undefined'
				for cat in entry.category
					categories.push(cat.$.term)
		categories
		
					
	app.get '/api/atom', (req, res) ->
		res.header({'Content-Type': 'application/xml' })      
		res.render 'atom/atom', 
			title : 'Blog entries'
			host	: app.host

	app.get '/api/atom/categories', (req, res) ->
		res.header({'Content-Type': 'application/xml' })
		category.all (result)->
			res.render 'atom/categories',
				categories:result
				
	processFeeds = (req, res)->
		format = ''
		if req.headers['accept'] && req.headers['accept'].indexOf('text/html') >= 0
			format = 'sanitize'
		# list all posts when logged in, e.g. MarsEdit
		request.validate req, (result)=>
			promise
			if result isnt null
				promise = blog.findAll format
			else
				promise = blog.find format

			promise.then (result)=>
				res.header({'Content-Type': 'application/xml' })
				res.render 'atom/feeds',
					host		:	app.host
					title		:	result.title
					updated		:	result.updated
					posts		:	result.posts

	app.get '/api/atom/feeds', processFeeds
						
	app.post '/api/atom/feeds', authorize, (req, res) -> 
		promise = parseBody(req.rawBody)
		promise.then (result) ->
			blogPromise = blog.createPost
				title   	: 	result.title
				body    	: 	result.content
				publish		: 	result.publish
				author 		: 	settings.author
				categories 	: 	result.categories
			blogPromise.then (result)->
				location = app.host + 'api/atom/entries/' + result._id
				res.header({
					'Content-Type'	: req.headers['content-type'] 
					'Location'			: location
					})
				# post is created.
				res.statusCode = 201
				res.render 'atom/entries', 
					post : result
					host  : app.host 
			
	app.get '/api/atom/entries/:id', (req, res) ->
		blog.findPostById req.params.id, (result)->
				if req.headers['accept'] && req.headers['accept'].indexOf('text/html') >=0
					result.body = helper.htmlEscape(settings.format(result.body))
				
				result.title = result.title.trim()
			
				res.header({'Content-Type': 'application/atom+xml' })
				res.render 'atom/entries', 
					post : result
					host  : app.host

	app.put '/api/atom/entries/:id',authorize, (req, res)->
			promise = parseBody(req.rawBody)
			promise.then (result) ->
				promise = blog.updatePost 
					id			:	req.params.id
					title		:	result.title
					body		:	result.content
					publish		: 	result.publish
					categories	: 	result.categories
				promise.then (result)->
					res.render 'atom/entries', 
						post : result
						host : app.host

	app.delete '/api/atom/entries/:id', authorize , (req, res)->
		blog.deletePost req.params.id,()->
			res.end()

	app.get '/rsd.xml', (req, res) ->
					res.header({'Content-Type': 'application/xml' })
					res.render 'rsd',
						host : app.host
						engine : settings.engine
	
	app.post '/api/atom/images', authorize, (req, res)->
		slug = req.headers['slug']
		promise = media.create	
			id:new ObjectId()
			slug:slug
			type:req.headers['content-type']
			body:req.rawBody
		promise.then (result)->
				res.statusCode = 201
				res.render 'atom/media',
					media : result
					host  : app.host
					author:	settings.author

	app.get '/images/:year/:month/:slug', (req, res)->
		url = util.format("%s/%s/%s", req.params.year, req.params.month, req.params.slug)
		promise = media.get url 
		promise.then (result)->
			if result isnt null
				res.send(result.data)
		, (err) ->
			res.send(404, err.message)

	app.get '/:year/:month/:title', (req, res) ->
		link = util.format("%s/%s/%s", req.params.year, req.params.month, req.params.title)
		# get the most recent posts, to be displayed on the right
		recent = []
		promise = blog.findMostRecent()
		promise.then (result)=>
			recent = result
			promise = blog.findPost link
			promise.then (result)->
				if result isnt null
					result.host = app.host
					result.recent = recent
					res.render 'post', result
				else
					promise = blog.hasPostMoved link
					promise.then (result)->
						if result isnt null
							res.redirect(301, "/" + JSON.parse(result.content).permaLink)
						else		
							res.end("Invalid url or could not find the post")

	app.get '/', (req, res) ->
		recent = []
		promise = blog.findMostRecent()
		promise.then (result)=>
			recent = result
			promise = blog.find 'list'
			promise.then (result)->
				result.host = app.host
				result.recent = recent
				res.render 'index', result
				
module.exports = routes
