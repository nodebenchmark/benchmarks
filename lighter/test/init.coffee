
class TestBase
	constructor: ()->	
		path = require 'path'
		settings = (require path.join(__dirname, '../settings'))()
		require(path.join(__dirname, '../schema'))(settings.mongoose)
		@blog = (require '../modules/blog')(settings)
		@category = (require '../modules/category')(settings)
		@media = (require '../modules/media')(settings)
		user = require('../modules/user')(settings)
		user.init (data)->
			console.log 'Initializing of user %s is completed',  data.username
	blog:@blog
	category:@category
	media:@media

module.exports = new TestBase()