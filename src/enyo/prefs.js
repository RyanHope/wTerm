enyo.kind({

	name: 'Preferences',
	kind: enyo.Control,

	prefs: {},

	published: {
		defaults: {
			firstUse: false,
			rootLaunchPoint: null,
			setupLaunchPoint: null,
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
			kbdLayout: (enyo.fetchDeviceInfo().keyboardAvailable || enyo.fetchDeviceInfo().keyboardSlider) ? 'phone_aux' : 'qwerty_us',
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
	},

	lsvar: enyo.fetchAppId() + '_prefs',

	constructor: function() {
	    this.inherited(arguments);
		this.load();
	},

	load: function() {
		if (enyo.fetchDeviceInfo().platformVersionMajor == 1) {
			var prefs = enyo.getCookie(this.lsvar)
			if (prefs)
				this.prefs = enyo.mixin(this.defaults, enyo.json.parse(prefs))
			else
				this.prefs = this.defaults
				enyo.setCookie(this.lsvar, enyo.json.stringify(this.prefs))
		} else {
			if (localStorage && localStorage[this.lsvar])
				this.prefs = enyo.mixin(this.defaults, enyo.json.parse(localStorage[this.lsvar]));
			else if (localStorage) {
				this.prefs = this.defaults;
				localStorage[this.lsvar] = enyo.json.stringify(this.prefs);
			}
			else this.error('no localStorage?');
		}
	},

	save: function(prefs) {
		this.prefs = prefs;
		if (enyo.fetchDeviceInfo().platformVersionMajor == 1) {
			enyo.setCookie(this.lsvar, enyo.json.stringify(this.prefs))
			return true
		} else {
			if (localStorage) {
				localStorage[this.lsvar] = enyo.json.stringify(this.prefs);
				return true;
			} else {
				this.error('no localStorage?');
				return false;
			}
		}
	},

	set: function(key, value) {
		if (typeof(this.prefs[key]) != "undefined") {
			this.prefs[key] = value;
			return this.save(this.prefs);
		} else {
			return false;
		}
	},

	get: function(item) {
		if (this.prefs[item]) return this.prefs[item];
		else return false;
	},

});

enyo.application.p = new Preferences();
