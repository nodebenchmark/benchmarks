module.exports = ()->
	util = require('util')
	class Helper
		linkify: (source) ->
			source = source.toLowerCase()
			source = source.replace(/^\s*|\s*$/g, '')
			source = source.replace(/:+/g, '')
			source = source.replace(/[?#&!()+=]+/g, '') 
			source = source.replace(/\s+/g, '-')
			util.format("%s/%s", @.dateNow(), source)
			
		dateNow: ()->
			dateNow = new Date()
			mm = dateNow.getMonth() + 1
			if mm < 10
				mm = '0' + mm
			yy = dateNow.getFullYear()
			util.format('%s/%s',yy, mm)
		htmlEscape:(html)->
			html = html.replace(/&(?!\w+;)/g, '&amp;')
									.replace(/</g, '&lt;')
									.replace(/>/g, '&gt;')
									.replace(/"/g, '&quot;')
			html
			
	return new Helper()