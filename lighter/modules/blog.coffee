module.exports = (settings)->
	class Blog
		constructor: (settings) ->
			@settings = settings
			path = require('path')
			@blog = settings.mongoose.model 'blog'
			@post = settings.mongoose.model 'post'
			@helper = (require __dirname + '/../helper')()
			@category = (require __dirname + '/category')(settings)
			@media = (require __dirname + '/media')(settings)
			@map = settings.mongoose.model 'map'

		create:()->
			promise = new @settings.Promise()
			@blog.findOne url : @settings.url, (err, data)=>
				if data isnt null
					if data.title isnt @settings.title
						data.title = @settings.title
						data.save (err, data)->
							promise.resolve(data)
					else
						promise.resolve(data)
				else
					blog = new @blog
						url		: @settings.url
						title	: @settings.title
						updated : @settings.updated
					blog.save (err, data) ->
						promise.resolve(data)

			return promise

		createPost:(obj)=>
			promise = new @settings.Promise()
			@.create().then (result)=>
				if result isnt null
					@_post
						id 	: result._id
						post: obj
						, (data)=>
							promise.resolve data
			return promise
		
		find:(format, filter)->
			promise = new @settings.Promise()
			@blog.findOne url : @settings.url, (err, data) =>
				if err!= null
					throw err.message
				blog = data

				if typeof filter is 'undefined'
					filter =
						id		:	blog._id
						publish	:	true
				else 
					filter.id = blog._id

				@post.find(filter).sort({date: -1}).exec (err, data)=>
					posts = []
					for post in data 
						if format is 'sanitize' || 'list'
							post.body = @settings.format(post.body) 
							if format is 'list'
								post.body = post.body.match(/<p>(.*?)<\/p>/g)[0]
						post.title = post.title.trim()
						posts.push post

					promise.resolve({
						id 		: blog._id
						title 	: blog.title
						updated : blog.updated
						posts 	: posts
					})
			return promise

		findAll:(format)->
			return @find format, {}
			
		findMostRecent:()->
			promise = new @settings.Promise
			@blog.findOne url:@settings.url, (err, data) =>
				@post.find({id : data._id, publish : true}).sort({date: -1}).limit(5).exec (err, data)=>
						recent = []
						for post in data
								recent.push({
									title 		: post.title
									permaLink :	post.permaLink
								})
						promise.resolve(recent)
			return promise
			
		findPost: (permaLink)->
			promise = new @settings.Promise
			@blog.findOne url: @settings.url, (err, data) =>
				blog = data
				@post.findOne 
					id : blog._id 
					permaLink: permaLink,(err, data)=>
						if err != null or data == null
							promise.resolve(null)
							return
						summary = data.body.trim()	
						data.summary = summary.substring(0, summary.indexOf('\n'))
						data.body = @settings.format(data.body)
						promise.resolve({
							title	:	blog.title
							post	:	data
						})
			return promise
	
		hasPostMoved: (permaLink)->
			promise = new @settings.Promise
			@map.findOne
				permaLink : permaLink, (err, data)->
					promise.resolve(data)
			return promise

		findPostById: (id, callback)->
			@post.findOne 
				_id : id, (err, data)=>
					callback(data)
	
		updatePost: (post)->
			promise = new @settings.Promise
			@post.findOne
				_id : post.id, (err, data)=>
					previous =
						id 			: data._id
						title 		: data.title
						permaLink 	: data.permaLink
						body  		: data.body

					data.body = post.body
				
					if data.title isnt post.title 
						data.title = post.title
						data.permaLink = @helper.linkify post.title
				
					data.categories = post.categories
					data.publish = post.publish

					if (data.categories)
						for category in data.categories
							@category.refresh category

					data.save (err, data)=>
						post = data
						permaLink = previous.permaLink
						@map.findOne
							permaLink : permaLink, (err, data)=>
								if data is null
									map = new @map
									map.permaLink = permaLink
								else
									map = data

								map.content = JSON.stringify({
										id : post.id,
										title : post.title,
										permaLink : post.permaLink,
										body : post.body	
									})

								map.save (err, data)->
									if err is null
										promise.resolve(post)
									else
										throw err
			return promise
	
		deletePost: (id, callback)->
			@post.remove
				_id : id, ()=>
					@map.remove ()->
					callback()
	
		delete: (callback) ->  
			@blog.find url : @settings.url, (err, data) =>
				for blog in data
					@post.remove id : blog._id, ()=>
						@blog.remove url : @settings.url
			@category.clear () ->
			@media.clear ()->
			@map.remove ()->
				callback()

		_post: (obj, callback) ->
			post = obj.post
			permaLink = @helper.linkify(post.title)

			if typeof post.permaLink != 'undefined'
				permaLink = post.permaLink

			postSchema = new @post
					id 			: obj.id
					title 		: post.title
					permaLink	: permaLink
					author 		: post.author
					body 		: post.body
					publish 	: post.publish
					date		: new Date()		
					categories 	: post.categories
			postSchema.save (err, data) =>
					if err != null
						callback(err.message)
					if (data.categories)
						for category in data.categories
							@category.refresh category, (id)->
					callback(data)
					return
					
	new Blog settings