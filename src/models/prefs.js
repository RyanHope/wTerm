function Prefs() {
	
	this.defaults = {
		firstUse: false,
		exec: 'login -f wterm',
		exhibition: 'cmatrix',
		rootpassOK: false,
		launchParamsOK: false,
		bufferlines: 1000,
		fontSize: 12,
		showVKB: true,
		inputScheme: 'XFree86 xterm',
		inputSchemes: {
			'X11R6 xterm': [
				'\\x1b[11~',
				'\\x1b[12~',
				'\\x1b[13~',
				'\\x1b[14~',
				'\\x1b[15~',
				'\\x1b[17~',
				'\\x1b[18~',
				'\\x1b[19~',
				'\\x1b[20~',
				'\\x1b[21~',
				'\\x1b[23~',
				'\\x1b[24~',
			],
			'XFree86 xterm': [
				'\\x1bOP',
				'\\x1bOQ',
				'\\x1bOR',
				'\\x1bOS',
				'\\x1b[15~',
				'\\x1b[17~',
				'\\x1b[18~',
				'\\x1b[19~',
				'\\x1b[20~',
				'\\x1b[21~',
				'\\x1b[23~',
				'\\x1b[24~',
			],
			'rxvt': [
				'\\x1b[11~',
				'\\x1b[12~',
				'\\x1b[13~',
				'\\x1b[14~',
				'\\x1b[15~',
				'\\x1b[17~',
				'\\x1b[18~',
				'\\x1b[19~',
				'\\x1b[20~',
				'\\x1b[21~',
				'\\x1b[23~',
				'\\x1b[24~',
			]
		},
		colorScheme: 'Linux Colors',
		kbdLayout: 'default',
		colorSchemes: {
			'Linux Colors': [
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
			],
			'Black on White': [
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
				[0,0,0],
				[255,255,255],
				[0,0,0],
				[255,255,255],
			],
			'Black on Light Yellow': [
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
				[0,0,0],
				[255,255,221],
				[0,0,0],
				[255,255,221],
			],
			'Black on Random Light': [
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
				[0,0,0],
				null,
				[0,0,0],
				null,
			],
			'White on Black': [
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
				[255,255,255],
				[0,0,0],
				[255,255,255],
				[0,0,0],
			],
			'Green on Black': [
				[0,0,0],
				[250,75,75],
				[24,178,24],
				[178,104,24],
				[92,167,251],
				[255,30,255],
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
				[24,240,24],
				[0,0,0],
				[24,240,24],
				[0,0,0],
			],
			'Dark Pastels': [
				[63,63,63],
				[112,80,80],
				[96,180,138],
				[223,175,143],
				[154,184,215],
				[220,140,195],
				[140,208,211],
				[220,220,204],
				[112,144,128],
				[220,163,163],
				[114,213,163],
				[240,223,175],
				[148,191,243],
				[236,147,211],
				[147,224,227],
				[255,255,255],
				[220,220,204],
				[44,44,44],
				[220,220,204],
				[44,44,44],
			]
		}
	}
	
	for (key in this.defaults) {
		if (this.get(key)==null)
			this.set(key, this.defaults[key])
		else if (key == 'colorSchemes') {
			var colorSchemes = this.get(key)
			for (var colorScheme in this.defaults[key])
				colorSchemes[colorScheme] = this.defaults[key][colorScheme]
			this.set(key, colorSchemes)
		}
		else if (key == 'inputSchemes') {
			var inputSchemes = this.get(key)
			for (var inputScheme in this.defaults[key])
				inputSchemes[inputScheme] = this.defaults[key][inputScheme]
			this.set(key, inputSchemes)
		}
	}
	
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

Prefs.prototype.reset = function() {
	for (key in this.defaults) {
		this.set(key, this.defaults[key])
	}
}