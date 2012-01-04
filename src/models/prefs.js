function Prefs() {
	
	this.defaults = {
		fontSize: 12,
		colors: [
			[0,0,0],
			[178,24,24],
			[24,178,24],
			[178,104,24],
			[24,24,178],
			[178,24,178],
			[24,178,178],
			[178,178,178],
			[104,104,104],
			[255,84,84],
			[84,255,84],
			[255,255,84],
			[84,84,255],
			[255,84,255],
			[84,255,255],
			[255,255,255],
			[178,178,178],
			[0,0,0],
			[255,255,255],
			[104,104,104],
		]
	}
	
	for (key in this.defaults)
		if (this.get(key)==null)
			this.set(key, this.defaults[key])
	
}

Prefs.prototype.set = function(key, value) {
	localStorage.setItem(key, JSON.stringify(value))
}

Prefs.prototype.get = function(key) {
	return JSON.parse(localStorage.getItem(key))
}

Prefs.prototype.clear = function() {
	localStorage.clear()
}