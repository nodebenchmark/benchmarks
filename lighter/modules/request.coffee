module.exports = (settings)->
	class Request
		constructor:(settings)->
			@settings = settings
			@user = require(__dirname + '/user')(settings)

		validate:(req, callback)->  
			authValue = req.headers['authorization'] 
			if typeof(authValue) != 'undefined' and authValue.indexOf('Basic') >= 0
				buff = new Buffer(authValue.split(' ')[1] ,'base64')
				content = buff.toString('utf8')
				if content.indexOf(':') >= 0
					credentials = content.split(':')
					username = credentials[0]
					password = credentials[1]
					@user.find username, password, (data)->
						callback(data)
						return
					return
			else
				callback(null)
			
	new Request(settings)
          