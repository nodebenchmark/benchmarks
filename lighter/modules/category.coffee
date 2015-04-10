module.exports = (settings)->
	class Category
		constructor:(settings)->
			@category = settings.mongoose.model 'category'
			@helper = (require __dirname + '/../helper')()
			@settings = settings
			@cats = []
	
		refresh:(category)->
			promise = new @settings.Promise

			title = category.trim()
			link = @helper.linkify(title)

			@category.findOne title:title, (err, data)=>				
				if data is null
					category = new @category
						title 		: title
						permaLink	: link
					category.save (err, data) ->   
						 	 promise.resolve(data)
				else
			  		promise.resolve(data)

			return promise				
																
		all:(callback)->
			@category.find (err, data)->
				callback(data)

		clear:(callback)->
			@category.remove ()->
				callback()
					
	new Category(settings)