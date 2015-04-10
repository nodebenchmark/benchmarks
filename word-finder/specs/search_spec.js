var words = require('../lib/words');
var	_ = require('underscore');

describe('matching underscore', function(){
	it('matches _h', function(){
		var find = '_h';
		var result = words.search(find).result;

		expect(result.length).toBe(7);

		_.each(result, function(match){
			expect(match.length).toBe(2);
			expect(match.indexOf('H')).toBe(1);
		}, result);
	});

	it('matches h_', function(){
		var find = 'h_';
    var		result = words.search(find).result;

		_.each(result, function(match){
			expect(match.length).toBe(2);
			expect(match.indexOf('H')).toBe(0);
		}, result);
	});
});
