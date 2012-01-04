enyo.kind({
	name: 'wi.Header',
	kind: enyo.Control,
	published: {
		type:			'',
		icon:			'',
		title:			'',
		version:		'',
		tagline:		'Random Taglines Are Awesome',
		date:			[],
		random:			[]
	},
	_selected:	false,
	_type:		'',
	_icon:		'',
	_title:		'',
	_version:	'',
	_tagline:	'',
	
	components: [
		{kind: 'PageHeader', components: [
			{name: 'header', kind: 'HFlexBox', className: 'wi-header', components: [
				{name: 'icon', className: 'icon', kind: 'Image'},
				{className: 'text', flex: 1, components: [
					{kind: 'HFlexBox', components: [
						{name: 'title', className: 'title', allowHtml: true},
						{name: 'version', flex: 1, className: 'version'},
					]},
					{name: 'tagline', className: 'tagline', allowHtml: true},
				]},
			]},
		]},
		{name:'updateIcon', kind: enyo.PalmService, service: enyo.palmServices.application,
			method: 'updateLaunchPointIcon', onSuccess: 'updatedIcon', onFailure: 'updateIconFailed'},
	],
	
	rendered: function() {
		if (!this._selected) {
			var d			= this.getDate()
			var r			= this.getRandom();
			this._type		= d.type    || r.type    || this.type;
			this._icon		= d.icon    || r.icon    || this.icon    || enyo.fetchAppInfo().icon;
			this._title		= d.title   || r.title   || this.title   || enyo.fetchAppInfo().title;
			this._version	= d.version || r.version || this.version || 'v' + enyo.fetchAppInfo().version;
			this._tagline	= d.tagline || r.tagline || this.tagline || '&nbsp;';
			this._selected	= true;
		}
		if (this._type && !this.$.header.hasClass(this._type))
			this.$.header.addClass(this._type);
		this.$.icon.setSrc(this._icon);
		if (this._icon != enyo.fetchAppInfo().icon)
			this.$.updateIcon.call({icon: this._icon, launchPointId: enyo.fetchAppInfo().id + '_default'});
		this.$.title.setContent(this._title);
		this.$.version.setContent(this._version);
		this.$.tagline.setContent(this._tagline);
	},
	
	setIcon: function(icon) {
		if (icon) this.$.icon.setSrc(icon);
		else this.$.icon.setSrc(this._icon);
	},
	
	getDate: function() {
		if (this.date.length == 0) return false;
		var date  = new Date();
		var day   = date.getDate();
		var month = date.getMonth() + 1;
		var year  = date.getFullYear();
		for (var d = 0; d < this.date.length; d++) {
			if ((this.date[d].day   && this.date[d].day   == day)   &&
				(this.date[d].month && this.date[d].month == month) &&
				(this.date[d].year  && this.date[d].year  == year))
				return this.date[d];
		}
		for (var d = 0; d < this.date.length; d++) {
			if ((this.date[d].day   && this.date[d].day   == day)   &&
				(this.date[d].month && this.date[d].month == month) &&
				(!this.date[d].year))
				return this.date[d];
		}
		return false;
	},
	getRandom: function() {
		var w = 0;
		if (this.random.length == 0) return false;
		for (var r = 0; r < this.random.length; r++) {
			if (!this.random[r].weight) this.random[r].weight = 1;
			w += this.random[r].weight;
		}
		var ran = Math.floor(Math.random() * w) + 1;
		for (var r = 0; r < this.random.length; r++) {
			if (ran <= this.random[r].weight)
				return this.random[r];
			else
				ran -= this.random[r].weight;
		}
		return this.random[0];
	},
	
});